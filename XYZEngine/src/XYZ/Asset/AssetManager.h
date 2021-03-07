#pragma once
#include "XYZ/Core/GUID.h"
#include "XYZ/Utils/StringUtils.h"
#include "Asset.h"

namespace XYZ {

	class AssetManager
	{
	public:

		template<typename T, typename... Args>
		static Ref<T> CreateAsset(const std::string& filename, AssetType type, size_t directoryHandle, Args&&... args)
		{
			static_assert(std::is_base_of<Asset, T>::value, "CreateAsset only works for types derived from Asset");

			auto& directory = GetAsset<Directory>(directoryHandle);

			Ref<T> asset = Ref<T>::Create(std::forward<Args>(args)...);
			asset->Type = type;
			asset->FilePath = directory->FilePath + "/" + filename;
			asset->FileName = Utils::RemoveExtension(Utils::GetFilename(asset->FilePath));
			asset->Extension = Utils::GetFilename(filename);
			asset->ParentDirectory = directoryHandle;
			asset->Handle = std::hash<std::string>()(asset->FilePath);
			asset->IsLoaded = true;
			s_LoadedAssets[asset->Handle] = asset;

			return asset;
		}

		template<typename T>
		static Ref<T> GetAsset(size_t assetHandle, bool loadData = true)
		{
			XYZ_ASSERT(s_LoadedAssets.find(assetHandle) != s_LoadedAssets.end(),"");
			Ref<Asset> asset = s_LoadedAssets[assetHandle];

			if (!asset->IsLoaded && loadData)
			{
				//asset = AssetSerializer::LoadAssetData(asset);
			}
			return asset.As<T>();
		}

	private:
		std::unordered_map<GUID, Ref<Asset>> s_LoadedAssets;
	};
}