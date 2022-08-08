#pragma once
#include "Asset.h"
#include "AssetSerializer.h"

namespace XYZ {
	class AssetImporter
	{
	public:
		static void Init();
		static void Serialize(const AssetMetadata& metadata, WeakRef<Asset> asset);
		static void Serialize(const Ref<Asset>& asset);
		static bool TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset);

	private:
		static std::array<Scope<AssetSerializer>, ToUnderlying(AssetType::NumTypes)> s_Serializers;
	};
}