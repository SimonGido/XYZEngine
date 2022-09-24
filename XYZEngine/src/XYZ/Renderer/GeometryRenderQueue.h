#pragma once
#include "RenderPass.h"
#include "Renderer2D.h"
#include "Mesh.h"
#include "RenderCommandBuffer.h"
#include "StorageBufferSet.h"
#include "VertexBufferSet.h"
#include "PipelineCompute.h"
#include "MaterialInstance.h"

#include "XYZ/Asset/Renderer/MaterialAsset.h"

#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/Components.h"

namespace XYZ {
	struct GeometryRenderQueue
	{
		struct SpriteDrawData
		{
			uint32_t  TextureIndex;
			glm::vec4 TexCoords;
			glm::vec4 Color;
			glm::mat4 Transform;
		};

		struct BillboardDrawData
		{
			uint32_t  TextureIndex;
			glm::vec4 TexCoords;
			glm::vec4 Color;
			glm::vec3 Position;
			glm::vec2 Size;
		};

		struct SpriteDrawCommand
		{
			Ref<MaterialAsset>  Material;
			std::array<Ref<Texture2D>, Renderer2D::GetMaxTextures()> Textures;

			uint32_t       TextureCount = 0;

			uint32_t SetTexture(const Ref<Texture2D>& texture);

			std::vector<SpriteDrawData>		SpriteData;
			std::vector<BillboardDrawData>	BillboardData;
		};

		struct TransformData
		{
			glm::vec4 TransformRow[3];
		};
		using BoneTransforms = std::array<ozz::math::Float4x4, 60>;

		struct MeshDrawCommandOverride
		{
			Ref<MaterialInstance>  OverrideMaterial;
			glm::mat4			   Transform;
		};

		struct MeshDrawCommand
		{
			Ref<Mesh>					 Mesh;
			Ref<MaterialAsset>			 MaterialAsset;
			Ref<MaterialInstance>		 OverrideMaterial;
			Ref<Pipeline>				 Pipeline;
			uint32_t					 TransformInstanceCount = 0;

			std::vector<TransformData>	 TransformData;
			uint32_t					 TransformOffset = 0;

			std::vector<MeshDrawCommandOverride> OverrideCommands;
		};

		struct AnimatedMeshDrawCommandOverride
		{
			Ref<MaterialInstance>  OverrideMaterial;
			glm::mat4			   Transform;
			BoneTransforms		   BoneTransforms;
			uint32_t			   BoneTransformsIndex = 0;
		};

		struct AnimatedMeshDrawCommand
		{
			Ref<AnimatedMesh>			 Mesh;
			Ref<MaterialAsset>			 MaterialAsset;
			Ref<MaterialInstance>		 OverrideMaterial;
			Ref<Pipeline>				 Pipeline;
			uint32_t					 TransformInstanceCount = 0;

			std::vector<TransformData>	 TransformData;
			uint32_t					 TransformOffset = 0;

			std::vector<BoneTransforms>	 BoneData;
			uint32_t					 BoneTransformsIndex = 0;

			std::vector<AnimatedMeshDrawCommandOverride> OverrideCommands;
		};

		struct InstanceMeshDrawCommand
		{
			Ref<Mesh>			  Mesh;
			Ref<MaterialAsset>	  MaterialAsset;
			Ref<MaterialInstance> OverrideMaterial;
			Ref<Pipeline>		  Pipeline;
			glm::mat4			  Transform;

			std::vector<std::byte> InstanceData;
			uint32_t			   InstanceCount = 0;
			uint32_t			   InstanceOffset = 0;
		};

		struct SpriteKey
		{
			SpriteKey(const AssetHandle& matHandle)
				: MaterialHandle(matHandle)
			{}

			bool operator<(const SpriteKey& other) const
			{
				return (MaterialHandle < other.MaterialHandle);
			}

			AssetHandle MaterialHandle;
		};
		struct BatchMeshKey
		{
			AssetHandle MeshHandle;
			AssetHandle MaterialHandle;

			BatchMeshKey(AssetHandle meshHandle, AssetHandle materialHandle)
				: MeshHandle(meshHandle), MaterialHandle(materialHandle) {}

			bool operator<(const BatchMeshKey& other) const
			{
				if (MeshHandle < other.MeshHandle)
					return true;

				return (MeshHandle == other.MeshHandle) && (MaterialHandle < other.MaterialHandle);
			}
		};


		std::map<SpriteKey, SpriteDrawCommand> SpriteDrawCommands;
		std::map<SpriteKey, SpriteDrawCommand> BillboardDrawCommands;

		std::map<BatchMeshKey, MeshDrawCommand>			MeshDrawCommands;
		std::map<BatchMeshKey, AnimatedMeshDrawCommand>	AnimatedMeshDrawCommands;
		std::map<BatchMeshKey, InstanceMeshDrawCommand>	InstanceMeshDrawCommands;
	};
}