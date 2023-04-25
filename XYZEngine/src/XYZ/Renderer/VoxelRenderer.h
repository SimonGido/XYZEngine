#pragma once
#include "PipelineCompute.h"
#include "StorageBufferSet.h"
#include "UniformBufferSet.h"
#include "StorageBufferAllocator.h"
#include "VoxelMesh.h"
#include "Material.h"

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
		glm::vec4 LightDirection;
		glm::vec4 LightColor;

		uint32_t MaxTraverses = 512;


		static constexpr uint32_t Binding = 16;
		static constexpr uint32_t Set = 0;
	};


	struct SSBOVoxels
	{
		static constexpr uint32_t MaxVoxels = 512 * 512 * 512;
					
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

		float		VoxelSize;
		Bool32		OriginInside; // Check if we start raycasting from inside grid

		Padding<4> Padding;
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

		void SubmitMesh(const Ref<VoxelMesh>& mesh, const glm::mat4& transform, float voxelSize);
		void SubmitMesh(const Ref<VoxelMesh>& mesh, const glm::mat4& transform, const uint32_t* keyFrames, float voxelSize);

		void OnImGuiRender();

		Ref<Image2D> GetFinalPassImage() const { return m_OutputTexture->GetImage(); }
	private:

		struct VoxelCommandModel
		{
			VoxelModel Model;
			uint32_t SubmeshIndex;
		};

		struct VoxelDrawCommand
		{
			Ref<VoxelMesh> Mesh;
			std::vector<VoxelCommandModel> Models;
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

		struct Statistics
		{
			uint32_t ModelCount = 0;
			uint32_t CulledModels = 0;
		};

	private:
		bool submitSubmesh(const VoxelSubmesh& submesh, VoxelDrawCommand& drawCommand, const glm::mat4& transform, float voxelSize, uint32_t submeshIndex);

		void clear();
		void render();

		void updateViewportSize();
		void updateUniformBufferSet();
		void prepareDrawCommands();

		void createRaymarchPipeline();

		MeshAllocation& createMeshAllocation(const Ref<VoxelMesh>& mesh);	

		void reallocateVoxels(const Ref<VoxelMesh>& mesh, MeshAllocation& allocation);
	private:
		Ref<PrimaryRenderCommandBuffer> m_CommandBuffer;
		Ref<PipelineCompute>	m_RaymarchPipeline;
		Ref<Material>			m_RaymarchMaterial;

		Ref<PipelineCompute>	m_ClearPipeline;
		Ref<Material>			m_ClearMaterial;

		Ref<StorageBufferSet>	m_StorageBufferSet;
		Ref<UniformBufferSet>	m_UniformBufferSet;

		Ref<StorageBufferAllocator> m_VoxelStorageAllocator;
		Ref<StorageBufferAllocator> m_ColorStorageAllocator;

		Ref<Texture2D>			m_OutputTexture;
		Ref<Texture2D>			m_DepthTexture;

		UBVoxelScene			m_UBVoxelScene;
		SSBOVoxels				m_SSBOVoxels;
		SSBOVoxelModels			m_SSBOVoxelModels;
		SSBOColors				m_SSBOColors;

		Math::Frustum			m_Frustum;

		glm::ivec2				m_ViewportSize;
		glm::ivec2				m_WorkGroups;
		bool				    m_ViewportSizeChanged = false;

		Statistics				m_Statistics;
	

		std::map<AssetHandle, VoxelDrawCommand> m_DrawCommands;

		std::unordered_map<AssetHandle, MeshAllocation> m_MeshAllocations;
		std::unordered_map<AssetHandle, MeshAllocation> m_LastFrameMeshAllocations;


		std::vector<UpdatedAllocation> m_UpdatedAllocations;

		struct GPUTimeQueries
		{
			uint32_t GPUTime = 0;

			static constexpr uint32_t Count() { return sizeof(GPUTimeQueries) / sizeof(uint32_t); }
		};
		GPUTimeQueries m_GPUTimeQueries;
	};

}
