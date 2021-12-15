#include "stdafx.h"
#include "AssetSerializer.h"

#include "XYZ/Scene/SceneSerializer.h"

namespace XYZ {
	void SceneAssetSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		SceneSerializer serializer(asset.As<Scene>());
		serializer.Serialize(metadata.FilePath.string());
	}
	bool SceneAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset, bool allocate) const
	{
		SceneSerializer serializer(asset.As<Scene>());
		asset = serializer.Deserialize(metadata.FilePath.string());
		return true;
	}
	void MaterialAssetSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
	}
	bool MaterialAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset, bool allocate) const
	{
		if (allocate)
		{
			// asset = Material::Create() ...
		}
		else
		{
			
		}
		return false;
	}
	void TextureAssetSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
	}
	bool TextureAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset, bool allocate) const
	{
		if (allocate)
		{
			// asset = Texture2D::Create() ...
		}
		else
		{
			TextureProperties properties{}; // Load properties from file
			Texture2D::Create(asset.As<Texture2D>(), "blabla", properties);
		}
		return false;
	}
}