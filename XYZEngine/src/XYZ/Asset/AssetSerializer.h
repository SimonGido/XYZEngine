#pragma once
#include "Asset.h"

namespace XYZ {

	class AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const = 0;
		virtual bool TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const = 0;
	};


	class SceneAssetSerializer : public AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const override;
		virtual bool TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
	};


	class ShaderAssetSerializer : public AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const override;
		virtual bool TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
	};

	class MaterialAssetSerializer : public AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const override;
		virtual bool TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
	};


	class TextureAssetSerializer : public AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const override;
		virtual bool TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
	};

	class MeshSourceAssetSerializer : public AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const override;
		virtual bool TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
	};

	class SkeletonAssetSerializer : public AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const override;
		virtual bool TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
	};

	class AnimationAssetSerializer : public AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const override;
		virtual bool TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
	};

	class MeshAssetSerializer : public AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const override;
		virtual bool TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
	};

	class AnimatedMeshAssetSerializer : public AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const override;
		virtual bool TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
	};

	class PrefabAssetSerializer : public AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const override;
		virtual bool TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
	};


	class SubTextureSerializer : public AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetadata& metadata, const WeakRef<Asset>& asset) const override;
		virtual bool TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
	};
}