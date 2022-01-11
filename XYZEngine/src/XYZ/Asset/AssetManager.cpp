#include "stdafx.h"
#include "AssetManager.h"

#include <filesystem>

#include <yaml-cpp/yaml.h>

#include "XYZ/Core/Ref/RefAllocator.h"


namespace XYZ
{
	MemoryPool												AssetManager::s_Pool = MemoryPool(1024 * 1024 * 10);
	std::unordered_map<AssetHandle, WeakRef<Asset>>			AssetManager::s_LoadedAssets;
	std::unordered_map<AssetHandle, AssetMetadata>			AssetManager::s_AssetMetadata;
	std::unordered_map<std::filesystem::path, AssetHandle>	AssetManager::s_AssetHandleMap;
	std::shared_ptr<FileWatcher>							AssetManager::s_FileWatcher;
	
	static std::string s_Directory = "Assets";

	void AssetManager::Init()
	{
		AssetImporter::Init();
		processDirectory(s_Directory);
		std::wstring wdir = std::wstring(s_Directory.begin(), s_Directory.end());
		s_FileWatcher = std::make_shared<FileWatcher>(wdir);
		s_FileWatcher->AddListener<AssetFileListener>();
		s_FileWatcher->Start();
	}
	void AssetManager::Shutdown()
	{
		s_LoadedAssets.clear();
		s_FileWatcher->Stop();
	}

	void AssetManager::ReloadAsset(const std::filesystem::path& filepath)
	{
		const auto metadata = getMetadata(filepath);
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
		auto metadata = getMetadata(handle);
		XYZ_ASSERT(metadata, "Metadata does not exist");
		return *metadata;
	}

	const AssetMetadata& AssetManager::GetMetadata(const std::filesystem::path& filepath)
	{
		auto metadata = getMetadata(filepath);
		XYZ_ASSERT(metadata, "Metadata does not exist");
		return *metadata;
	}

	const std::string& AssetManager::GetDirectory()
	{
		return s_Directory;
	}
	
	AssetMetadata* AssetManager::getMetadata(const GUID& handle)
	{
		auto it = s_AssetMetadata.find(handle);
		if (it != s_AssetMetadata.end())
			return &it->second;

		return nullptr;
	}

	AssetMetadata* AssetManager::getMetadata(const std::filesystem::path& filepath)
	{
		auto it = s_AssetHandleMap.find(filepath);
		if (it != s_AssetHandleMap.end())
		{
			return &s_AssetMetadata.find(it->second)->second;
		}
		return nullptr;
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
			s_AssetHandleMap[filepath] = guid;
			AssetMetadata& metadata = s_AssetMetadata[guid];
			metadata.Handle = guid;
			metadata.FilePath = filePath.as<std::string>();
			metadata.Type = Utils::AssetTypeFromString(type.as<std::string>());
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
}