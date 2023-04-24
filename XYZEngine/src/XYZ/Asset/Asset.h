#pragma once
#include "XYZ/Core/Ref/Ref.h"
#include "XYZ/Core/GUID.h"
#include "XYZ/Core/Assert.h"

#include <filesystem>

namespace XYZ {

	enum class AssetType
	{
		None,
		Scene,
		Texture,
		SubTexture,
		Material,
		Shader,
		Font,
		Audio,
		Script,
		StaticMesh,
		AnimatedMesh,
		MeshSource,
		Animation,
		AnimationController,
		Skeleton,
		Prefab,
		Blueprint,
		ParticleSystemGPU,
		VoxelMeshSource,
		VoxelSourceMesh,
		NumTypes
	};

	enum class AssetFlag : uint16_t
	{
		None     = 0,
		Missing	 = BIT(0),
		Invalid	 = BIT(1),
		Reloaded = BIT(2),
		ReadOnly = BIT(3)
	};

	namespace Utils {

		inline AssetType AssetTypeFromString(const std::string& assetType)
		{
			if (assetType == "None") 					  return AssetType::None;
			if (assetType == "Scene")					  return AssetType::Scene;
			if (assetType == "Texture")					  return AssetType::Texture;
			if (assetType == "SubTexture")				  return AssetType::SubTexture;
			if (assetType == "Material")				  return AssetType::Material;
			if (assetType == "Shader")					  return AssetType::Shader;
			if (assetType == "Font")					  return AssetType::Font;
			if (assetType == "Audio")					  return AssetType::Audio;
			if (assetType == "Script")					  return AssetType::Script;
			if (assetType == "Mesh")					  return AssetType::StaticMesh;
			if (assetType == "AnimatedMesh")			  return AssetType::AnimatedMesh;
			if (assetType == "MeshSource")				  return AssetType::MeshSource;
			if (assetType == "Animation")				  return AssetType::Animation;
			if (assetType == "AnimationController")		  return AssetType::AnimationController;
			if (assetType == "Skeleton")				  return AssetType::Skeleton;
			if (assetType == "Prefab")					  return AssetType::Prefab;
			if (assetType == "Blueprint")				  return AssetType::Blueprint;
			if (assetType == "ParticleSystemGPU")		  return AssetType::ParticleSystemGPU;
			if (assetType == "VoxelMeshSource")			  return AssetType::VoxelMeshSource;
			if (assetType == "VoxelSourceMesh")			  return AssetType::VoxelSourceMesh;

			XYZ_ASSERT(false, "Unknown Asset Type");
			return AssetType::None;
		}
		inline const char* AssetTypeToString(AssetType assetType)
		{
			switch (assetType)
			{
			case XYZ::AssetType::None:							return "None";
			case XYZ::AssetType::Scene:		 					return "Scene";
			case XYZ::AssetType::Texture:						return "Texture";
			case XYZ::AssetType::SubTexture:					return "SubTexture";
			case XYZ::AssetType::Material:						return "Material";
			case XYZ::AssetType::Shader:						return "Shader";
			case XYZ::AssetType::Font:							return "Font";
			case XYZ::AssetType::Audio:							return "Audio";
			case XYZ::AssetType::Script:						return "Script";
			case XYZ::AssetType::StaticMesh:					return "Mesh";
			case XYZ::AssetType::AnimatedMesh:					return "AnimatedMesh";
			case XYZ::AssetType::MeshSource:					return "MeshSource";
			case XYZ::AssetType::Animation:						return "Animation";
			case XYZ::AssetType::AnimationController:			return "AnimationController";
			case XYZ::AssetType::Skeleton:						return "Skeleton";
			case XYZ::AssetType::Prefab:						return "Prefab";
			case XYZ::AssetType::Blueprint:						return "Blueprint";
			case XYZ::AssetType::ParticleSystemGPU:				return "ParticleSystemGPU";
			case XYZ::AssetType::VoxelMeshSource:				return "VoxelMeshSource";
			case XYZ::AssetType::VoxelSourceMesh:				return "VoxelSourceMesh";
			default:
				break;
			}

			XYZ_ASSERT(false, "Unknown Asset Type");
			return "None";
		}

		inline const char* AssetTypeToExtension(AssetType type)
		{
			switch (type)
			{
			case XYZ::AssetType::None:							return "none";
			case XYZ::AssetType::Scene:		 					return "scene";
			case XYZ::AssetType::Texture:						return "tex";
			case XYZ::AssetType::SubTexture:					return "subtex";
			case XYZ::AssetType::Material:						return "mat";
			case XYZ::AssetType::Shader:						return "shader";
			case XYZ::AssetType::Font:							return "font";
			case XYZ::AssetType::Audio:							return "audio";
			case XYZ::AssetType::Script:						return "cs";
			case XYZ::AssetType::StaticMesh:					return "mesh";
			case XYZ::AssetType::AnimatedMesh:					return "animmesh";
			case XYZ::AssetType::MeshSource:					return "meshsrc";
			case XYZ::AssetType::Animation:						return "anim";
			case XYZ::AssetType::AnimationController:			return "controller";
			case XYZ::AssetType::Skeleton:						return "skeleton";
			case XYZ::AssetType::Prefab:						return "prefab";
			case XYZ::AssetType::Blueprint:						return "blueprint";
			case XYZ::AssetType::ParticleSystemGPU:				return "particle";
			case XYZ::AssetType::VoxelMeshSource:				return "voxmeshsrc";
			case XYZ::AssetType::VoxelSourceMesh:				return "voxmesh";
			default:
				break;
			}

			XYZ_ASSERT(false, "Unknown Asset Type");
			return "None";
		}
	}

	using AssetHandle = GUID;

	class XYZ_API Asset : public RefCount
	{
	public:
		virtual ~Asset() = default;
	
		virtual AssetType GetAssetType() const = 0;
		const AssetHandle& GetHandle() const { return m_Handle; }
		
		bool IsValid() const;
		
		bool IsFlagSet(AssetFlag flag) const { return (uint16_t)flag & m_Flags; }
		void SetFlag(AssetFlag flag, bool value = true);
		
		
		static AssetType GetStaticType() { return AssetType::None; }
		static const char* GetExtension(AssetType type) { return Utils::AssetTypeToExtension(type); }
	private:
		AssetHandle m_Handle;
		uint16_t    m_Flags = 0;


		friend class AssetManager;
		friend class AssetImporter;
	};

	struct AssetMetadata
	{
		AssetHandle			  Handle;
		AssetType			  Type = AssetType::None;
		std::filesystem::path FilePath;
	};	
}