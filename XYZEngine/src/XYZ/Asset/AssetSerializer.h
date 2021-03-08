#pragma once
#include "Asset.h"



namespace XYZ {
	class AssetSerializer
	{
	public:
		template <typename T>
		static void SerializeAsset(Ref<T> asset)
		{
			static_assert(std::is_base_of<Asset, T>::value, "SerializeAsset only accepts types that inherit from Asset");
			Serialize<T>(asset);
		}

		static Ref<Asset> LoadAssetMeta(const std::string& filepath, const GUID& directoryHandle, AssetType type);
		
		static Ref<Asset> LoadAssetData(Ref<Asset> asset);
		

	private:
		template <typename T>
		static Ref<Asset> deserialize(Ref<Asset> asset);

		template <typename T>
		static void serialize(Ref<Asset> asset);
		
		static void loadMetaFile(Ref<Asset> asset);
		static void createMetaFile(Ref<Asset> asset);
	};
}