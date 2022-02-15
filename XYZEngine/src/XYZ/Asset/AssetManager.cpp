#include "stdafx.h"
#include "AssetManager.h"

#include <filesystem>

#include <yaml-cpp/yaml.h>

#include "XYZ/Core/Ref/RefAllocator.h"


namespace XYZ
{
	MemoryPool												AssetManager::s_Pool = MemoryPool(1024 * 1024 * 10);
	AssetRegistry											AssetManager::s_Registry;
	std::unordered_map<AssetHandle, WeakRef<Asset>>			AssetManager::s_LoadedAssets;
	std::unordered_map<AssetHandle, WeakRef<Asset>>			AssetManager::s_MemoryAssets;
	std::shared_ptr<FileWatcher>							AssetManager::s_FileWatcher;
	
	static std::string s_Directory = "Assets";

	void AssetManager::Init()
	{
		AssetImporter::Init();
		processDirectory(s_Directory);
		processDirectory("Resources");

		std::wstring wdir = std::wstring(s_Directory.begin(), s_Directory.end());
		s_FileWatcher = std::make_shared<FileWatcher>(wdir);
		
		s_FileWatcher->AddOnFileChange<&onFileChange>();
		s_FileWatcher->AddOnFileAdded<&onFileAdded>();
		s_FileWatcher->AddOnFileRemoved<&onFileRemoved>();
		s_FileWatcher->AddOnFileRenamed<&onFileRenamed>();
		s_FileWatcher->Start();
	}
	void AssetManager::Shutdown()
	{
		s_LoadedAssets.clear();
		s_MemoryAssets.clear();
		s_FileWatcher->Stop();
	}

	void AssetManager::SerializeAll()
	{
		for (const auto& [handle, asset] : s_LoadedAssets)
		{
			if (asset.IsValid())
			{
				const auto& metadata = GetMetadata(handle);
				AssetImporter::Serialize(metadata, asset);
			}
		}
	}

	void AssetManager::Serialize(const AssetHandle& assetHandle)
	{
		auto it = s_LoadedAssets.find(assetHandle);
		if (it != s_LoadedAssets.end() && it->second.Raw())
		{
			const auto& metadata = GetMetadata(assetHandle);
			AssetImporter::Serialize(metadata, it->second);
		}
		else
		{
			XYZ_WARN("Trying to serialize asset that does not exist!");
		}
	}

	void AssetManager::ReloadAsset(const std::filesystem::path& filepath)
	{
		const auto metadata = s_Registry.GetMetadata(filepath);
		if (metadata)
		{
			Ref<Asset> asset = nullptr;
			bool loaded = AssetImporter::TryLoadData(*metadata, asset);
			if (!loaded)
			{
				XYZ_WARN("Could not load asset {}", filepath);
				return;
			}
			s_LoadedAssets[asset->GetHandle()] = asset;
		}
	}

	const AssetMetadata& AssetManager::GetMetadata(const AssetHandle& handle)
	{
		auto metadata = s_Registry.GetMetadata(handle);
		XYZ_ASSERT(metadata, "Metadata does not exist");
		return *metadata;
	}

	const AssetMetadata& AssetManager::GetMetadata(const std::filesystem::path& filepath)
	{
		auto metadata = s_Registry.GetMetadata(filepath);
		XYZ_ASSERT(metadata, "Metadata does not exist");
		return *metadata;
	}

	const std::string& AssetManager::GetDirectory()
	{
		return s_Directory;
	}
	
	bool AssetManager::Exist(const AssetHandle& handle)
	{
		return s_Registry.GetMetadata(handle) != nullptr;
	}


	void AssetManager::loadAssetMetadata(const std::filesystem::path& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());

		auto handle = data["Handle"];
		auto filePath = data["FilePath"];
		auto type = data["Type"];

		if (handle && filePath && type)
		{
			AssetHandle guid(handle.as<std::string>());
			AssetMetadata metadata;
			metadata.Handle = guid;
			metadata.FilePath = filePath.as<std::string>();
			metadata.Type = Utils::AssetTypeFromString(type.as<std::string>());
			s_Registry.StoreMetadata(metadata);
		}
		else
		{
			XYZ_WARN("Failed to load asset meta data {0}", filepath);
		}
	}

	void AssetManager::writeAssetMetadata(const AssetMetadata& metadata)
	{
		std::string filepath = metadata.FilePath.string();
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Handle" << YAML::Value << (std::string)metadata.Handle;
		out << YAML::Key << "FilePath" << YAML::Value << filepath;
		out << YAML::Key << "Type" << YAML::Value << Utils::AssetTypeToString(metadata.Type);
		out << YAML::EndMap;

		std::ofstream fout(filepath + ".meta");
		fout << out.c_str();
	}
	void AssetManager::processDirectory(const std::filesystem::path& path)
	{
		for (auto it : std::filesystem::directory_iterator(path))
		{
			if (it.is_directory())
			{
				processDirectory(it.path());
			}
			else if (Utils::GetExtension(it.path().string()) == "meta")
			{
				loadAssetMetadata(it.path());
			}
		}
	}
	void AssetManager::reloadAsset(const AssetMetadata& metadata, Ref<Asset>& asset)
	{
		AssetImporter::TryLoadData(metadata, asset);
	}
	void AssetManager::onFileChange(const std::wstring& path)
	{
		std::string strPath(path.begin(), path.end());
		std::replace(strPath.begin(), strPath.end(), '\\', '/');
		if (strPath.find(".meta") == std::string::npos)
			AssetManager::ReloadAsset("Assets/" + strPath + ".meta");
	}
	void AssetManager::onFileAdded(const std::wstring& path)
	{
	}
	void AssetManager::onFileRemoved(const std::wstring& path)
	{
	}
	void AssetManager::onFileRenamed(const std::wstring& path)
	{
	}
}