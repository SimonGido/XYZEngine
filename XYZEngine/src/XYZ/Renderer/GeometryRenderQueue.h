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
			Ref<Material>		  Material;
			Ref<MaterialInstance> MaterialInstance;

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
			uint32_t					 Count = 0;

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
			uint32_t					 Count = 0;
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

		struct SSBOState
		{
			uint32_t Size;
			uint32_t Offset;
		};


		struct IndirectComputeCommand
		{
			Ref<MaterialInstance>  OverrideMaterial;
			std::vector<std::byte> ComputeData;
			uint32_t			   ComputeResultSize;

			SSBOState			   IndirectCommandState;
			SSBOState			   ComputeDataState;
			SSBOState			   ComputeResultState;
		};

		struct IndirectComputeBatch
		{
			Ref<PipelineCompute>   Pipeline;
			Ref<MaterialAsset>	   MaterialCompute;

			std::vector<IndirectComputeCommand> Commands;
		};

		struct IndirectMeshDrawCommandOverride
		{
			Ref<Mesh>			  Mesh;
			Ref<MaterialInstance> OverrideMaterial;
			uint32_t			  ComputeDataSize;
			uint32_t			  ComputeResultSize;

			
			SSBOState			  IndirectCommandState;
			SSBOState			  ComputeDataState;
			SSBOState			  ComputeResultState;
		};

		struct IndirectMeshDrawCommand
		{
			Ref<MaterialAsset>	  MaterialAsset;
			Ref<Pipeline>		  Pipeline;

			std::vector<IndirectMeshDrawCommandOverride> OverrideCommands;
		};



		std::map<SpriteKey, SpriteDrawCommand> SpriteDrawCommands;
		std::map<SpriteKey, SpriteDrawCommand> BillboardDrawCommands;

		std::map<BatchMeshKey, MeshDrawCommand>			MeshDrawCommands;
		std::map<BatchMeshKey, AnimatedMeshDrawCommand>	AnimatedMeshDrawCommands;
		std::map<BatchMeshKey, InstanceMeshDrawCommand>	InstanceMeshDrawCommands;
		std::map<AssetHandle,  IndirectMeshDrawCommand>	IndirectDrawCommands;
		std::map<AssetHandle,  IndirectComputeBatch>	IndirectComputeCommands;
		
		void Clear()
		{
			SpriteDrawCommands.clear();
			BillboardDrawCommands.clear();
			MeshDrawCommands.clear();
			AnimatedMeshDrawCommands.clear();
			InstanceMeshDrawCommands.clear();
			IndirectDrawCommands.clear();
			IndirectComputeCommands.clear();
		}
	};
}