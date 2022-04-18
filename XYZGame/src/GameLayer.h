#pragma once

#include <XYZ.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "EditorCamera.h"
#include "MarchingCubes.h"

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

	
	private:
		void displayStats();

	private:
		Ref<Scene>			m_Scene;
		Ref<SceneRenderer>	m_SceneRenderer;
		
		EditorCamera		m_Camera;
	
		SceneEntity			m_TestQuadEntity;

		Ref<RenderCommandBuffer> m_CommandBuffer;
		Ref<RenderPass>			 m_RenderPass;
		Ref<Pipeline>			 m_Pipeline;
		Ref<Material>			 m_Material;
		Ref<MaterialInstance>	 m_MaterialInstance;
		Ref<StorageBufferSet>	 m_StorageBufferSet;

		Ref<UniformBufferSet>	 m_UniformBufferSet;

		struct UBScene
		{
			glm::mat4 ViewProjection{};
			glm::mat4 InverseView{};
			glm::mat4 CameraFrustum{};
			glm::vec4 CameraPosition{};
			glm::vec4 LightDirection{};
			glm::vec4 LightColor{};
		};
		UBScene m_SceneUB;

		float m_MaxDistance = 2.0f;
		float m_VoxelSize = 1.0f;
		static constexpr int VOXEL_GRID_SIZE = 5;

		float m_Time = 0.0f;
		int m_Voxels[VOXEL_GRID_SIZE][VOXEL_GRID_SIZE][VOXEL_GRID_SIZE];
	};
}