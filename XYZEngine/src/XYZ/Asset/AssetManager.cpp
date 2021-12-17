#include "stdafx.h"
#include "AssetManager.h"
#include "XYZ/Debug/MemoryPoolDebug.h"

#include <filesystem>

#include <yaml-cpp/yaml.h>

#include "XYZ/Core/Ref/RefAllocator.h"


namespace XYZ
{

	MemoryPool										AssetManager::s_Pool = MemoryPool(1024 * 1024 * 10);
	std::unordered_map<GUID, WeakRef<Asset>>		AssetManager::s_LoadedAssets;
	std::unordered_map<GUID, AssetMetadata>			AssetManager::s_AssetMetadata;
	std::unordered_map<std::filesystem::path, GUID>	AssetManager::s_AssetHandleMap;
	std::shared_ptr<FileWatcher>					AssetManager::s_FileWatcher;
	AssetFileListener*								AssetManager::s_FileListener;
	
	static std::string s_Directory = "Assets";

	void AssetManager::Init()
	{
		AssetImporter::Init();
		processDirectory(s_Directory);
		std::wstring wdir = std::wstring(s_Directory.begin(), s_Directory.end());
		s_FileWatcher = FileWatcher::Create(wdir);
		s_FileListener = new AssetFileListener();
		s_FileWatcher->AddListener(s_FileListener);
		s_FileWatcher->Start();
	}
	void AssetManager::Shutdown()
	{
		s_LoadedAssets.clear();
		s_FileWatcher->Stop();
		delete s_FileListener;
	}

	//void AssetManager::DisplayMemory()
	//{
	//	MemoryPoolDebug<1024 * 1024, true> memoryDebug;
	//	memoryDebug.SetContext(&s_Pool);
	//	memoryDebug.OnImGuiRender();
	//}


	void AssetManager::ReloadAsset(const std::filesystem::path& filepath)
	{
		const auto& metadata = GetMetadata(filepath);
		GetAsset<Asset>(metadata.Handle);
	}

	const AssetMetadata& AssetManager::GetMetadata(const GUID& handle)
	{
		return getMetadata(handle);
	}

	const AssetMetadata& AssetManager::GetMetadata(const std::filesystem::path& filepath)
	{
		auto it = s_AssetHandleMap.find(filepath);
		if (it != s_AssetHandleMap.end())
		{
			return s_AssetMetadata.find(it->second)->second;
		}
		XYZ_WARN("Meta data not found");
		return AssetMetadata();
	}

	const std::string& AssetManager::GetDirectory()
	{
		// TODO: insert return statement here
		return s_Directory;
	}

	AssetMetadata& AssetManager::getMetadata(const GUID& handle)
	{
		auto it = s_AssetMetadata.find(handle);
		if (it != s_AssetMetadata.end())
			return it->second;

		XYZ_WARN("Meta data not found");
		return AssetMetadata();
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
			GUID guid(handle.as<std::string>());
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
		AssetImporter::TryLoadData(metadata, asset, false);
	}
}