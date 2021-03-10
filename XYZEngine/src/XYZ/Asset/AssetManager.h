#pragma once
#include "XYZ/Core/GUID.h"
#include "XYZ/Utils/StringUtils.h"
#include "XYZ/Renderer/Shader.h"
#include "AssetSerializer.h"
#include "Asset.h"

namespace XYZ {


	class AssetManager
	{
	public:
		static void Init();
		static void Shutdown();

		static AssetType GetAssetTypeFromExtension(const std::string& extension);
		static GUID		 GetAssetHandle(const std::string& filepath);
		static GUID		 GetDirectoryHandle(const std::string& filepath);
		

		template<typename T, typename... Args>
		static Ref<T> CreateAsset(const std::string& filename, AssetType type, const GUID& directoryHandle, Args&&... args)
		{
			static_assert(std::is_base_of<Asset, T>::value, "CreateAsset only works for types derived from Asset");

			auto& directory = s_Directories[directoryHandle];
			Ref<T> asset = Ref<T>::Create(std::forward<Args>(args)...);
			asset->Type = type;
			asset->FilePath = directory.FilePath + "/" + filename;
			asset->FileName = Utils::RemoveExtension(Utils::GetFilename(asset->FilePath));
			asset->FileExtension = Utils::GetFilename(filename);
			asset->DirectoryHandle = directoryHandle;
			asset->Handle = GUID();
			asset->IsLoaded = true;
			s_LoadedAssets[asset->Handle] = asset;
			AssetSerializer::SerializeAsset(asset);

			return asset;
		}

		template<typename T>
		static Ref<T> GetAsset(const GUID& assetHandle, bool loadData = true)
		{
			XYZ_ASSERT(s_LoadedAssets.find(assetHandle) != s_LoadedAssets.end(),"");
			Ref<Asset> asset = s_LoadedAssets[assetHandle];

			if (!asset->IsLoaded && loadData)
			{
				asset = AssetSerializer::LoadAssetData(asset);
			}
			return asset.As<T>();
		}


	private:
		static void processDirectory(const std::string& path, AssetDirectory& directory);
		static void importAsset(const std::string& path);

	private:
		static std::unordered_map<GUID, Ref<Asset>> s_LoadedAssets;
		static std::unordered_map<GUID, AssetDirectory> s_Directories;
		static std::unordered_map<std::string, AssetType> s_AssetTypes;
	};
}