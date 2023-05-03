#pragma once
#include "PipelineCompute.h"
#include "StorageBufferSet.h"
#include "UniformBufferSet.h"
#include "StorageBufferAllocator.h"
#include "VoxelMesh.h"
#include "Material.h"

#include "XYZ/Scene/Scene.h"
#include "XYZ/Utils/DataStructures/ThreadQueue.h"
#include "XYZ/Asset/Renderer/VoxelMeshSource.h"

namespace XYZ {

	struct VoxelsSpecification
	{
		glm::vec4 Translation;
		glm::vec4 Rotation;

		uint32_t  MaxTraverse;
		uint32_t  Width;
		uint32_t  Height;
		uint32_t  Depth;
		float	  VoxelSize;
	};

	struct UBVoxelScene
	{
		glm::mat4 InverseProjection;
		glm::mat4 InverseView;
		glm::vec4 CameraPosition;
		glm::vec4 ViewportSize;
		
		// Light info
		DirectionalLight DirectionalLight;

		static constexpr uint32_t Binding = 16;
		static constexpr uint32_t Set = 0;
	};


	struct SSBOVoxels
	{
		static constexpr uint32_t MaxVoxels = 512 * 1024 * 1024; // Half gig
					
		uint8_t Voxels[MaxVoxels];

		static constexpr uint32_t Binding = 17;
		static constexpr uint32_t Set = 0;
	};



	struct VoxelModel
	{
		glm::mat4	InverseModelView;
		glm::mat4	Transform;
		glm::vec4	RayOrigin; // Origin of raycasting for this model

		uint32_t	VoxelOffset;
		uint32_t	Width;
		uint32_t	Height;
		uint32_t	Depth;
		uint32_t    ColorIndex;
		uint32_t    MaxTraverses;

		float		VoxelSize;
		Bool32		OriginInside; // Check if we start raycasting from inside grid
	};

	struct SSBOVoxelModels
	{
		static constexpr uint32_t MaxModels = 1024;
		uint32_t NumModels;
		Padding<12> Padding;
		VoxelModel Models[MaxModels];

		static constexpr uint32_t Binding = 18;
		static constexpr uint32_t Set = 0;
	};

	struct SSBOColors
	{
		static constexpr uint32_t MaxColors = 1024;

		uint32_t ColorPallete[MaxColors][256];

		static constexpr uint32_t Binding = 19;
		static constexpr uint32_t Set = 0;
	};


	struct VoxelRendererCamera
	{
		glm::mat4 ViewProjectionMatrix;
		glm::mat4 ViewMatrix;
		glm::mat4 Projection;
		glm::vec3 CameraPosition;
		Math::Frustum Frustum;
	};


	class XYZ_API VoxelRenderer : public RefCount
	{
	public:
		VoxelRenderer();

		void BeginScene(const VoxelRendererCamera& camera);
		void EndScene();
		
		void SetViewportSize(uint32_t width, uint32_t height);

		void SubmitMesh(const Ref<VoxelMesh>& mesh, const glm::mat4& transform);
		void SubmitMesh(const Ref<VoxelMesh>& mesh, const glm::mat4& transform, const uint32_t* keyFrames);
		void SubmitMesh(const Ref<VoxelMesh>& mesh, const glm::mat4& transform, bool cull);

		void SubmitEffect(const Ref<MaterialAsset>& material, glm::ivec2 workGroups, const PushConstBuffer& constants);

		void OnImGuiRender();

		uint32_t	 GetModelCount() const { return m_SSBOVoxelModels.NumModels; }
		Ref<Image2D> GetFinalPassImage() const;
	private:

		struct VoxelCommandModel
		{
			uint32_t ModelIndex;
			uint32_t SubmeshIndex;
		};

		struct VoxelDrawCommand
		{
			Ref<VoxelMesh> Mesh;
	
			std::vector<VoxelCommandModel> Models;
		};

		struct VoxelEffectInvocation
		{
			glm::ivec2 WorkGroups;
			PushConstBuffer Constants;
		};

		struct VoxelEffectCommand
		{
			Ref<MaterialAsset> Material;
			
			std::vector<VoxelEffectInvocation> Invocations;
		};

		struct MeshAllocation
		{
			StorageBufferAllocation VoxelAllocation;
			StorageBufferAllocation	ColorAllocation;

			std::vector<uint32_t> SubmeshOffsets;
		};
	
		struct UpdatedAllocation
		{
			StorageBufferAllocation VoxelAllocation;
			StorageBufferAllocation	ColorAllocation;
		};

		struct UpdatedSuballocation
		{
			uint32_t Offset;
			uint32_t Size;
		};

		struct Statistics
		{
			uint32_t ModelCount = 0;
			uint32_t CulledModels = 0;
		};

		struct SSGIValues
		{
			uint32_t SampleCount = 10;
			float	 IndirectAmount = 1.0f;
			float	 NoiseAmount = 2.0f;
			Bool32   Noise = true;
		};

	private:
		void submitSubmesh(const VoxelSubmesh& submesh, VoxelDrawCommand& drawCommand, const glm::mat4& transform, uint32_t submeshIndex);

		void clearPass();
		void effectPass();
		void renderPass();
		void ssgiPass();
		void snowPass();

		void updateViewportSize();
		void updateUniformBufferSet();
		void prepareDrawCommands();

		void createDefaultPipelines();
		Ref<PipelineCompute> getEffectPipeline(const Ref<MaterialAsset>& material);

		MeshAllocation& createMeshAllocation(const Ref<VoxelMesh>& mesh);	

		void reallocateVoxels(const Ref<VoxelMesh>& mesh, MeshAllocation& allocation);
		bool cullSubmesh(const VoxelSubmesh& submesh, const glm::mat4& transform) const;
	private:
		Ref<PrimaryRenderCommandBuffer> m_CommandBuffer;

		Ref<PipelineCompute>	m_RaymarchPipeline;
		Ref<Material>			m_RaymarchMaterial;

		Ref<PipelineCompute>	m_ClearPipeline;
		Ref<Material>			m_ClearMaterial;

		Ref<PipelineCompute>	m_SSGIPipeline;
		Ref<Material>			m_SSGIMaterial;

		Ref<StorageBufferSet>	m_StorageBufferSet;
		Ref<UniformBufferSet>	m_UniformBufferSet;

		// Test SNOW //	
		Ref<PipelineCompute>	m_SnowPipeline;
		Ref<Material>			m_SnowMaterial;
		///////////////


		Ref<StorageBufferAllocator> m_VoxelStorageAllocator;
		Ref<StorageBufferAllocator> m_ColorStorageAllocator;

		Ref<Texture2D>			m_OutputTexture;
		Ref<Texture2D>			m_DepthTexture;
		Ref<Texture2D>			m_SSGITexture;

		UBVoxelScene			m_UBVoxelScene;
		SSBOVoxels				m_SSBOVoxels;
		SSBOVoxelModels			m_SSBOVoxelModels;
		SSBOColors				m_SSBOColors;

		SSGIValues				m_SSGIValues;
		Math::Frustum			m_Frustum;

		glm::ivec2				m_ViewportSize;
		glm::ivec2				m_WorkGroups;
		bool				    m_ViewportSizeChanged = false;

		Statistics				m_Statistics;
	
		bool					m_UseSSGI = false;

		bool					m_Snow = false;
		uint32_t				m_SnowFramesDelay = 0;
		uint32_t				m_SnowFramesCounter = 0;

		std::map<AssetHandle, VoxelDrawCommand> m_DrawCommands;
		std::map<AssetHandle, VoxelEffectCommand> m_EffectCommands;

		std::unordered_map<AssetHandle, MeshAllocation> m_MeshAllocations;
		std::unordered_map<AssetHandle, MeshAllocation> m_LastFrameMeshAllocations;

		std::unordered_map<AssetHandle, Ref<PipelineCompute>> m_EffectPipelines;

		std::vector<UpdatedAllocation> m_UpdatedAllocations;
		std::vector<UpdatedSuballocation> m_UpdatedSuballocations;

		struct GPUTimeQueries
		{
			uint32_t GPUTime = 0;

			static constexpr uint32_t Count() { return sizeof(GPUTimeQueries) / sizeof(uint32_t); }
		};
		GPUTimeQueries m_GPUTimeQueries;
	};

}
