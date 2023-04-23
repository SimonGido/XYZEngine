#pragma once
#include "PipelineCompute.h"
#include "StorageBufferSet.h"
#include "UniformBufferSet.h"
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
				
		uint32_t Colors[256];
		uint8_t Voxels[MaxVoxels];

		static constexpr uint32_t Binding = 17;
		static constexpr uint32_t Set = 0;
	};

	struct VoxelModel
	{
		glm::mat4	InverseTransform;
		glm::mat4	Transform;
		uint32_t	VoxelOffset;
		uint32_t	Width;
		uint32_t	Height;
		uint32_t	Depth;
		float		VoxelSize;
		// TODO: model must specify width height and depth otherwise raytracing wont work
		// Even though voxel model is in memory, it is probably in the first layers of our 3d grid
		// Probably it will not be possible to draw them in single compute dispatch
		uint32_t Padding[3];
	};

	struct SSBOVoxelModels
	{
		static constexpr uint32_t MaxModels = 1024;
		uint32_t NumModels;
		uint32_t Padding[3];
		VoxelModel Models[MaxModels];

		static constexpr uint32_t Binding = 18;
		static constexpr uint32_t Set = 0;
	};

	class VoxelRenderer : public RefCount
	{
	public:
		VoxelRenderer();

		void BeginScene(const glm::mat4& viewProjectionMatrix, const glm::mat4& viewMatrix, const glm::mat4& projection, const glm::vec3& cameraPosition);
		void EndScene();
		
		void SetViewportSize(uint32_t width, uint32_t height);
		void SetColors(const std::array<uint32_t, 256>& colors);
		void SetColors(const std::array<VoxelColor, 256>& colors);

		void SubmitMesh(const Ref<VoxelMeshSource>& meshSource, const glm::mat4& transform, float voxelSize);

		void OnImGuiRender();

		Ref<Image2D> GetFinalPassImage() const { return m_OutputTexture->GetImage(); }
	private:
		void clear();
		void render();

		void updateViewportSize();
		void updateUniformBufferSet();
		void updateStorageBufferSet();

		void createRaymarchPipeline();
	private:
		struct VoxelDrawCommand
		{
			std::vector<VoxelModel> Models;
		};


	private:
		Ref<PrimaryRenderCommandBuffer> m_CommandBuffer;
		Ref<PipelineCompute>	m_RaymarchPipeline;
		Ref<Material>			m_RaymarchMaterial;

		Ref<PipelineCompute>	m_ClearPipeline;
		Ref<Material>			m_ClearMaterial;

		Ref<StorageBufferSet>	m_StorageBufferSet;
		Ref<UniformBufferSet>	m_UniformBufferSet;
		Ref<Texture2D>			m_OutputTexture;
		Ref<Texture2D>			m_DepthTexture;

		UBVoxelScene			m_UBVoxelScene;
		SSBOVoxels				m_SSBOVoxels;
		SSBOVoxelModels			m_SSBOVoxelModels;

		glm::ivec2				m_ViewportSize;
		bool				    m_ViewportSizeChanged = false;
		uint32_t				m_CurrentVoxelsCount;

		std::map<AssetHandle, VoxelDrawCommand> m_DrawCommands;

		struct GPUTimeQueries
		{
			uint32_t GPUTime = 0;

			static constexpr uint32_t Count() { return sizeof(GPUTimeQueries) / sizeof(uint32_t); }
		};
		GPUTimeQueries m_GPUTimeQueries;
	};

}
