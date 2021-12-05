#include "stdafx.h"
#include "AssetManager.h"
#include "XYZ/Debug/MemoryPoolDebug.h"

#include <filesystem>

#include <yaml-cpp/yaml.h>

#include "XYZ/Core/Ref/RefAllocator.h"


namespace XYZ
{

	MemoryPool<1024 * 1024, true>							 AssetManager::s_Pool;
	std::unordered_map<GUID, WeakRef<Asset>>				 AssetManager::s_LoadedAssets;
	std::unordered_map<std::filesystem::path, AssetMetadata> AssetManager::s_AssetMetadata;

	void AssetManager::Init()
	{
		AssetImporter::Init();
		processDirectory("Assets");
	}
	void AssetManager::Shutdown()
	{
		s_LoadedAssets.clear();
	}

	//void AssetManager::DisplayMemory()
	//{
	//	MemoryPoolDebug<1024 * 1024, true> memoryDebug;
	//	memoryDebug.SetContext(&s_Pool);
	//	memoryDebug.OnImGuiRender();
	//}


	const AssetMetadata& AssetManager::GetMetadata(const GUID& handle)
	{
		return getMetadata(handle);
	}

	const AssetMetadata& AssetManager::GetMetadata(const std::filesystem::path& filepath)
	{
		auto it = s_AssetMetadata.find(filepath);
		if (it != s_AssetMetadata.end())
			return it->second;
		XYZ_WARN("Meta data not found");
		return AssetMetadata();
	}

	AssetMetadata& AssetManager::getMetadata(const GUID& handle)
	{
		for (auto& [filepath, metadata] : s_AssetMetadata)
		{
			if (metadata.Handle == handle)
				return metadata;
		}
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
			AssetMetadata& metadata = s_AssetMetadata[filepath];
			metadata.Handle = GUID(handle.as<std::string>());
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
}