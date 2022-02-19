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
		Mesh,
		AnimatedMesh,
		MeshSource,
		Animation,
		Animator,
		NumTypes
	};

	enum class AssetFlag : uint16_t
	{
		None     = 0,
		Missing  = BIT(0),
		Invalid  = BIT(1),
		ReadOnly = BIT(2)
	};

	namespace Utils {

		inline AssetType AssetTypeFromString(const std::string& assetType)
		{
			if (assetType == "None") 			  return AssetType::None;
			if (assetType == "Scene")			  return AssetType::Scene;
			if (assetType == "Texture")			  return AssetType::Texture;
			if (assetType == "SubTexture")		  return AssetType::SubTexture;
			if (assetType == "Material")		  return AssetType::Material;
			if (assetType == "Shader")			  return AssetType::Shader;
			if (assetType == "Font")			  return AssetType::Font;
			if (assetType == "Audio")			  return AssetType::Audio;
			if (assetType == "Script")			  return AssetType::Script;
			if (assetType == "Mesh")			  return AssetType::Mesh;
			if (assetType == "AnimatedMesh")	  return AssetType::AnimatedMesh;
			if (assetType == "MeshSource")		  return AssetType::MeshSource;
			if (assetType == "Animation")		  return AssetType::Animation;
			if (assetType == "Animator")		  return AssetType::Animator;
			

			XYZ_ASSERT(false, "Unknown Asset Type");
			return AssetType::None;
		}
		inline const char* AssetTypeToString(AssetType assetType)
		{
			switch (assetType)
			{
			case XYZ::AssetType::None:				return "None";
			case XYZ::AssetType::Scene:		 		return "Scene";
			case XYZ::AssetType::Texture:			return "Texture";
			case XYZ::AssetType::SubTexture:		return "SubTexture";
			case XYZ::AssetType::Material:			return "Material";
			case XYZ::AssetType::Shader:			return "Shader";
			case XYZ::AssetType::Font:				return "Font";
			case XYZ::AssetType::Audio:				return "Audio";
			case XYZ::AssetType::Script:			return "Script";
			case XYZ::AssetType::Mesh:				return "Mesh";
			case XYZ::AssetType::AnimatedMesh:		return "AnimatedMesh";
			case XYZ::AssetType::MeshSource:		return "MeshSource";
			case XYZ::AssetType::Animation:			return "Animation";
			case XYZ::AssetType::Animator:			return "Animator";
			default:
				break;
			}

			XYZ_ASSERT(false, "Unknown Asset Type");
			return "None";
		}
	}

	using AssetHandle = GUID;

	class Asset : public RefCount
	{
	public:
		virtual ~Asset() = default;
	
		const AssetHandle& GetHandle() const { return m_Handle; }

		static AssetType GetStaticType() { return AssetType::None; }

		bool IsFlagSet(AssetFlag flag) const { return (uint16_t)flag & m_Flags; }
		void SetFlag(AssetFlag flag, bool value = true)
		{
			if (value)
				m_Flags |= (uint16_t)flag;
			else
				m_Flags &= ~(uint16_t)flag;
		}
	private:
		AssetHandle m_Handle;
		uint16_t    m_Flags;

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