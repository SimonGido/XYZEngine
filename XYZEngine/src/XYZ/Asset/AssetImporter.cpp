#include "stdafx.h"
#include "AssetImporter.h"

#include "XYZ/Renderer/Texture.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/Font.h"
#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Renderer/Renderer.h"

#include "XYZ/Scene/SceneSerializer.h"
#include "XYZ/Utils/FileSystem.h"
#include "XYZ/Utils/YamlUtils.h"
#include "AssetManager.h"

#include <yaml-cpp/yaml.h>


#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>


namespace XYZ {

	std::array<Scope<AssetSerializer>, ToUnderlying(AssetType::NumTypes)> AssetImporter::s_Serializers;

	void AssetImporter::Init()
	{
		s_Serializers[ToUnderlying(AssetType::Scene)] = CreateScope<SceneAssetSerializer>();
		s_Serializers[ToUnderlying(AssetType::Texture)] = CreateScope<TextureAssetSerializer>();
		s_Serializers[ToUnderlying(AssetType::Material)] = CreateScope<MaterialAssetSerializer>();
		s_Serializers[ToUnderlying(AssetType::Shader)] = CreateScope<ShaderAssetSerializer>();
		s_Serializers[ToUnderlying(AssetType::MeshSource)] = CreateScope<MeshSourceAssetSerializer>();
		s_Serializers[ToUnderlying(AssetType::Mesh)] = CreateScope<MeshAssetSerializer>();
		s_Serializers[ToUnderlying(AssetType::AnimatedMesh)] = CreateScope<AnimatedMeshAssetSerializer>();
		s_Serializers[ToUnderlying(AssetType::SubTexture)] = CreateScope<SubTextureSerializer>();
		s_Serializers[ToUnderlying(AssetType::Prefab)] = CreateScope<PrefabAssetSerializer>();
		s_Serializers[ToUnderlying(AssetType::Animation)] = CreateScope<AnimationAssetSerializer>();
		s_Serializers[ToUnderlying(AssetType::Skeleton)] = CreateScope<SkeletonAssetSerializer>();
	}

	void AssetImporter::Serialize(const AssetMetadata& metadata, WeakRef<Asset> asset)
	{
		if (!s_Serializers[ToUnderlying(metadata.Type)])
		{
			XYZ_CORE_WARN("There is no importer for assets of type {0}", Utils::AssetTypeToString(metadata.Type));
			return;
		}
		s_Serializers[ToUnderlying(metadata.Type)]->Serialize(metadata, asset);
	}

	void AssetImporter::Serialize(const Ref<Asset>& asset)
	{
		const AssetMetadata& metadata = AssetManager::GetMetadata(asset->GetHandle());
		Serialize(metadata, asset);
	}

	bool AssetImporter::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset)
	{
		if (!s_Serializers[ToUnderlying(metadata.Type)])
		{
			XYZ_CORE_WARN("There is no importer for assets of type {0}", Utils::AssetTypeToString(metadata.Type));
			return false;
		}

		bool result = s_Serializers[ToUnderlying(metadata.Type)]->TryLoadData(metadata, asset);
		if (result)
			asset->m_Handle = metadata.Handle;
		return result;
	}

}