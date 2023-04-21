#pragma once
#include "PipelineCompute.h"
#include "StorageBufferSet.h"
#include "UniformBufferSet.h"
#include "Material.h"

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

		static constexpr uint32_t Binding = 16;
		static constexpr uint32_t Set = 0;
	};

	struct SSBOVoxels
	{
		static constexpr uint32_t MaxVoxels = 1024 * 1024 * 32;
		
		uint32_t Voxels[MaxVoxels];

		static constexpr uint32_t Binding = 17;
		static constexpr uint32_t Set = 0;
	};

	class VoxelRenderer : public RefCount
	{
	public:
		VoxelRenderer();

		void BeginScene(const glm::mat4& viewProjectionMatrix, const glm::mat4& viewMatrix, const glm::mat4& projection, const glm::vec3& cameraPosition);
		void EndScene();
		void SetViewportSize(uint32_t width, uint32_t height);


		void SubmitVoxels(const VoxelsSpecification& spec, uint32_t* voxels);

		void OnImGuiRender();

		Ref<Image2D> GetFinalPassImage() const { return m_OutputTexture->GetImage(); }
	private:
		void render();

		void updateViewportSize();
		void updateUniformBufferSet();
		void updateStorageBufferSet();

		void createRaymarchPipeline();
	private:
		struct VoxelDrawCommand
		{
			glm::mat4			Transform;
			uint32_t			MaxTraverse;
			uint32_t			Width;
			uint32_t			Height;
			uint32_t			Depth;
			float				VoxelSize;
			uint32_t			VoxelOffset;
			uint32_t			VoxelCount;
		};


	private:
		Ref<PrimaryRenderCommandBuffer> m_CommandBuffer;
		Ref<PipelineCompute>	m_RaymarchPipeline;
		Ref<Material>			m_RaymarchMaterial;

		Ref<StorageBufferSet>	m_StorageBufferSet;
		Ref<UniformBufferSet>	m_UniformBufferSet;
		Ref<Texture2D>			m_OutputTexture;

		UBVoxelScene			m_UBVoxelScene;
		SSBOVoxels				m_SSBOVoxels;

		glm::ivec2				m_ViewportSize;
		bool				    m_ViewportSizeChanged = false;
		uint32_t				m_CurrentVoxelsCount;

		std::vector<VoxelDrawCommand> m_DrawCommands;

	};

}
