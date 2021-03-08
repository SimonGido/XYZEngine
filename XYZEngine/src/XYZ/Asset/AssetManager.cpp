#include "stdafx.h"
#include "AssetManager.h"

#include <filesystem>

#include <yaml-cpp/yaml.h>

namespace XYZ {
	std::unordered_map<GUID, Ref<Asset>> AssetManager::s_LoadedAssets;
	std::unordered_map<std::string, AssetType> AssetManager::s_AssetTypes;

	void AssetManager::Init()
	{
		s_AssetTypes["xyz"] = AssetType::Scene;
		s_AssetTypes["tex"] = AssetType::Texture;
		s_AssetTypes["font"] = AssetType::Font;
		s_AssetTypes["subtex"] = AssetType::SubTexture;
		s_AssetTypes["mat"] = AssetType::Material;
		s_AssetTypes["cs"] = AssetType::Script;

		processDirectory("Assets");
	}
	void AssetManager::Shutdown()
	{
	}
	AssetType AssetManager::GetAssetTypeFromExtension(const std::string& extension)
	{
		return s_AssetTypes[extension];
	}
	void AssetManager::processDirectory(const std::string& path)
	{
		for (auto it : std::filesystem::directory_iterator(path))
		{
			if (it.is_directory())
				processDirectory(it.path().string());
			else
				importAsset(it.path().string());
		}
	}
	void AssetManager::importAsset(const std::string& path)
	{
		std::string extension = Utils::GetExtension(path);
		if (extension == "meta")
			return;
		
		AssetType type = s_AssetTypes[extension];
		Ref<Asset> asset = AssetSerializer::LoadAssetMeta(path, GUID(), type);
		if (s_LoadedAssets.find(asset->Handle) != s_LoadedAssets.end())
		{
			if (s_LoadedAssets[asset->Handle]->IsLoaded)
			{
				asset = AssetSerializer::LoadAssetData(asset);
			}
		}
		s_LoadedAssets[asset->Handle] = asset;
	}
}