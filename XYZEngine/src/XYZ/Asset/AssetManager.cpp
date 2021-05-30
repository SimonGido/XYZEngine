#include "stdafx.h"
#include "AssetManager.h"

#include <filesystem>

#include <yaml-cpp/yaml.h>



namespace XYZ
{

	std::unordered_map<GUID, Ref<Asset>> AssetManager::s_LoadedAssets;
	std::unordered_map<GUID, AssetDirectory> AssetManager::s_Directories;
	std::unordered_map<std::string, AssetType> AssetManager::s_AssetTypes;
	MemoryPool AssetManager::s_Pool = MemoryPool(1024 * 1024);

	void AssetManager::Init()
	{
		s_AssetTypes["xyz"]    = AssetType::Scene;
		s_AssetTypes["tex"]    = AssetType::Texture;
		s_AssetTypes["font"]   = AssetType::Font;
		s_AssetTypes["subtex"] = AssetType::SubTexture;
		s_AssetTypes["mat"]    = AssetType::Material;
		s_AssetTypes["shader"] = AssetType::Shader;
		s_AssetTypes["cs"]     = AssetType::Script;
		s_AssetTypes["skm"]    = AssetType::SkeletalMesh;

		RefAllocator::Init(&s_Pool);
		AssetDirectory newDirectory;
		newDirectory.FilePath = "Assets";
		s_Directories[newDirectory.Handle] = newDirectory;
		processDirectory("Assets", newDirectory);
	}
	void AssetManager::Shutdown()
	{
		s_LoadedAssets.clear();
	}

	AssetType AssetManager::GetAssetTypeFromExtension(const std::string& extension)
	{
		auto it = s_AssetTypes.find(extension);
		if (it != s_AssetTypes.end())
			return it->second;
		return AssetType::None;
	}

	GUID AssetManager::GetAssetHandle(const std::string& filepath)
	{
		for (auto& [id, asset] : s_LoadedAssets)
		{
			if (asset->FilePath == filepath)
				return id;
		}

		XYZ_ASSERT(false, "");
		return GUID();
	}

	GUID AssetManager::GetDirectoryHandle(const std::string& filepath)
	{
		for (auto& [id, dir] : s_Directories)
		{
			if (dir.FilePath == filepath)
				return id;
		}

		XYZ_ASSERT(false, "");
		return GUID();
	}

	std::vector<Ref<Asset>> AssetManager::FindAssetsByType(AssetType type)
	{
		std::vector<Ref<Asset>> assets;
		for (auto& [id, asset] : s_LoadedAssets)
		{
			if (asset->Type == type)
				assets.push_back(asset);
		}
		return assets;
	}

	bool AssetManager::IsValidExtension(const std::string& extension)
	{
		return s_AssetTypes.find(extension) != s_AssetTypes.end();
	}


	void AssetManager::processDirectory(const std::string& path, AssetDirectory& directory)
	{
		for (auto it : std::filesystem::directory_iterator(path))
		{
			if (it.is_directory())
			{
				AssetDirectory newDirectory;
				newDirectory.FilePath = it.path().string();
				std::replace(newDirectory.FilePath.begin(), newDirectory.FilePath.end(), '\\', '/');
				newDirectory.ParentHandle = directory.ParentHandle;
				directory.SubDirectoryHandles.push_back(newDirectory.Handle);
				s_Directories[newDirectory.Handle] = newDirectory;

				processDirectory(it.path().string(), s_Directories[newDirectory.Handle]);
			}
			else
			{
				importAsset(it.path().string());
			}
		}
	}
	void AssetManager::importAsset(const std::string& path)
	{
		std::string extension = Utils::GetExtension(path);
		if (extension == "meta")
			return;
		if (s_AssetTypes.find(extension) == s_AssetTypes.end())
			return;

		AssetType type = s_AssetTypes[extension];
		Ref<Asset> asset = AssetSerializer::LoadAssetMeta(path, GUID(), type);
		if (s_LoadedAssets.find(asset->Handle) != s_LoadedAssets.end())
		{
			if (s_LoadedAssets[asset->Handle]->IsLoaded)
			{
				asset = AssetSerializer::LoadAsset(asset);
			}
		}
		s_LoadedAssets[asset->Handle] = asset;
	}
}