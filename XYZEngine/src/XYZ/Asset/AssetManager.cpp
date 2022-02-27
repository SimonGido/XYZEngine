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
	
	static std::filesystem::path s_Directory = "Assets";

	void AssetManager::Init()
	{
		AssetImporter::Init();
		processDirectory(s_Directory);
		processDirectory("Resources");

		std::wstring wdir = s_Directory.wstring();
		s_FileWatcher = std::make_shared<FileWatcher>(wdir);
		
		s_FileWatcher->AddOnFileChanged<&onFileChange>();
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
			XYZ_CORE_WARN("Trying to serialize asset that does not exist!");
		}
	}

	void AssetManager::Update(Timestep ts)
	{
		s_FileWatcher->ProcessChanges();
	}
	std::vector<AssetMetadata> AssetManager::FindAllMetadata(AssetType type)
	{
		std::vector<AssetMetadata> result;
		for (auto& [handle, asset] : s_LoadedAssets)
		{
			auto metadata = s_Registry.GetMetadata(handle);
			if (metadata->Type == type)
			{
				result.push_back(*metadata);
			}
		}
		return result;
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
				XYZ_CORE_WARN("Could not load asset {}", filepath);
				return;
			}
			auto it = s_LoadedAssets.find(metadata->Handle);
			if (it != s_LoadedAssets.end())
			{
				it->second->SetFlag(AssetFlag::Reloaded);
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

	const std::filesystem::path& AssetManager::GetAssetDirectory()
	{
		return s_Directory;
	}
	
	bool AssetManager::Exist(const AssetHandle& handle)
	{
		return s_Registry.GetMetadata(handle) != nullptr;
	}

	bool AssetManager::Exist(const std::filesystem::path& filepath)
	{
		return s_Registry.GetMetadata(filepath) != nullptr;
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
			XYZ_CORE_WARN("Failed to load asset meta data {0}", filepath);
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


	static std::filesystem::path s_RenamedFileOldPath;

	void AssetManager::onFileChange(FileWatcher::ChangeType type, const std::filesystem::path& path)
	{
		if (type == FileWatcher::ChangeType::Modified)
		{
			if (AssetManager::Exist(path))
			{
				AssetManager::ReloadAsset(path);
			}
		}
		else if (type == FileWatcher::ChangeType::Added)
		{

		}
		else if (type == FileWatcher::ChangeType::Removed)
		{
			auto metadata = *s_Registry.GetMetadata(path);
			s_Registry.RemoveMetadata(metadata.Handle);
			auto it = s_LoadedAssets.find(metadata.Handle);
			if (it != s_LoadedAssets.end())
				s_LoadedAssets.erase(it);
		}
		else if (type == FileWatcher::ChangeType::RenamedOld)
		{
			s_RenamedFileOldPath = path;
		}
		else if (type == FileWatcher::ChangeType::RenamedNew)
		{
			const auto ptrMetadata = s_Registry.GetMetadata(s_RenamedFileOldPath);;
			if (ptrMetadata)
			{
				auto metadata = *ptrMetadata;
				s_Registry.RemoveMetadata(metadata.Handle);

				FileSystem::Rename(s_RenamedFileOldPath.string() + ".meta", Utils::GetFilename(path.string()));

				metadata.FilePath = path;
				s_Registry.StoreMetadata(metadata);
				writeAssetMetadata(metadata);
			}
		}
	}
}