#pragma once
#include "Camera.h"
#include "RenderPass.h"
#include "Renderer2D.h"
#include "Mesh.h"
#include "RenderCommandBuffer.h"
#include "StorageBufferSet.h"
#include "PipelineCompute.h"
#include "MaterialInstance.h"

#include "XYZ/Asset/MaterialAsset.h"

#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/Components.h"

namespace XYZ {
	class MeshRenderer
	{
	public:



	private:
		struct TransformData
		{
			glm::vec4 TransformRow[3];
		};
		using BoneTransforms = std::array<glm::mat4, 60>;

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
			glm::mat4			  Transform;

			std::vector<std::byte> InstanceData;
			uint32_t			   InstanceCount = 0;
			uint32_t			   InstanceOffset = 0;
		};
		struct Queue
		{

		};

	};
}