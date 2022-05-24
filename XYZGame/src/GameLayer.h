#pragma once

#include <XYZ.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "EditorCamera.h"
#include "MarchingCubes.h"
#include "Perlin.h"



namespace XYZ {
	class GameLayer : public Layer
	{
	public:
		GameLayer();
		virtual ~GameLayer() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;
		virtual void OnImGuiRender() override;

	private:
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onWindowResize(WindowResizeEvent& event);
		bool onKeyPress(KeyPressedEvent& event);
		bool onKeyRelease(KeyReleasedEvent& event);

		void generateVoxels();
	private:
		void displayStats();

		struct HitResult
		{
			enum Side { Top, Bottom, Left, Right, Front, Back, NumSides };
			uint32_t HitVoxelIndex;
			uint32_t X, Y, Z;
			Side	 HitSide = NumSides;
		};

		void voxelPreview();
		bool rayMarch(const glm::vec3& rayOrig, const glm::vec3& rayDir, HitResult& result);
		std::pair<glm::vec3, glm::vec3> castRay(float mx, float my) const;
	private:
		EditorCamera		m_Camera;
	
		Ref<PrimaryRenderCommandBuffer> m_CommandBuffer;


		Ref<RenderPass>				    m_RenderPass;
		Ref<Pipeline>				    m_Pipeline;


		Ref<PipelineCompute>			m_SDFPipeline;
		Ref<Material>					m_SDFMaterial;
		Ref<MaterialInstance>			m_SDFMaterialInstance;


		Ref<Material>				    m_Material;
		Ref<MaterialInstance>		    m_MaterialInstance;
		Ref<StorageBufferSet>		    m_StorageBufferSet;
									    
		Ref<UniformBufferSet>		    m_SceneBufferSet;

		static constexpr int VOXEL_GRID_SIZE = 200;

		struct UBScene
		{
			glm::mat4 ViewProjection{};
			glm::mat4 InverseView{};
			glm::mat4 CameraFrustum{};
			glm::vec4 CameraPosition{};
			glm::vec4 LightDirection{-0.2f, -1.4f, -1.5f, 1.0f};
			glm::vec4 LightColor{1.0f, 1.0f, 1.0f, 1.0f};
			glm::vec4 BackgroundColor{ 0.6f, 0.7f, 1.0f, 1.0f };

			glm::vec4 ChunkPosition = glm::vec4(0.0f);
			uint32_t  MaxTraverse = 128;
			uint32_t  Width = VOXEL_GRID_SIZE;
			uint32_t  Height = VOXEL_GRID_SIZE;
			uint32_t  Depth = VOXEL_GRID_SIZE;
			float	  VoxelSize = 1.0f;
			float	  Padding[3];
		};
		UBScene m_SceneUB;

		struct Voxel
		{
			uint32_t Color;
		};

		std::vector<Voxel> m_Voxels;

		uint32_t m_NumUpdates = 3;

		int m_Seed = 10;
		int m_Octaves = 4;
		int m_Height = 16;
		float m_Frequency = 1.0f;

		glm::vec4 m_Color = glm::vec4(1.0f);

		uint32_t m_LastPreviewVoxel = VOXEL_GRID_SIZE * VOXEL_GRID_SIZE * VOXEL_GRID_SIZE;

		std::atomic_bool m_Generating = false;
		bool m_QueuedGenerate = true;
	};
}