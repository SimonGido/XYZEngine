#include "stdafx.h"
#include "AssetImporter.h"

#include "XYZ/Renderer/Texture.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/Font.h"
#include "XYZ/Renderer/SkeletalMesh.h"
#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Animation/Animation.h"
#include "XYZ/Renderer/Renderer.h"

#include "XYZ/Scene/SceneSerializer.h"
#include "XYZ/Utils/FileSystem.h"
#include "XYZ/Utils/YamlUtils.h"
#include "AssetManager.h"

#include <yaml-cpp/yaml.h>


#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>


namespace XYZ {

	static YAML::Emitter& ToVec2(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}
	
	static YAML::Emitter& ToVec3(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}
	
	static YAML::Emitter& ToVec4(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	static YAML::Emitter& ToQuat(YAML::Emitter& out, const glm::quat& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.w << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	std::array<Scope<AssetSerializer>, ToUnderlying(AssetType::NumTypes)> AssetImporter::s_Serializers;

	void AssetImporter::Init()
	{
		s_Serializers[ToUnderlying(AssetType::Scene)] = CreateScope<SceneAssetSerializer>();
		s_Serializers[ToUnderlying(AssetType::Texture)] = CreateScope<TextureAssetSerializer>();
	}

	void AssetImporter::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset)
	{
		if (!s_Serializers[ToUnderlying(metadata.Type)])
		{
			XYZ_WARN("There is no importer for assets of type {0}", Utils::AssetTypeToString(metadata.Type));
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
			XYZ_WARN("There is no importer for assets of type {0}", Utils::AssetTypeToString(metadata.Type));
			return false;
		}

		bool result = s_Serializers[ToUnderlying(metadata.Type)]->TryLoadData(metadata, asset);
		if (result)
			asset->m_Handle = metadata.Handle;
		return result;
	}

}