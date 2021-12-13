#pragma once

#include <XYZ.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>



namespace XYZ {
	
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() override;

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

		void renderOverlay();

		std::pair<glm::vec3, glm::vec3> cameraToAABB(const TransformComponent& transform, const SceneCamera& camera) const;
	private:
		void displayStats();

	private:	
		Ref<Scene>					m_Scene;
		Ref<SceneRenderer>			m_SceneRenderer;
		
		Ref<RenderCommandBuffer>	m_CommandBuffer;
		Ref<Renderer2D>				m_OverlayRenderer2D;
		Ref<Texture2D>				m_CameraTexture;
		Ref<Material>				m_QuadMaterial;
		Ref<Material>				m_LineMaterial;
		Editor::EditorCamera*		m_EditorCamera = nullptr;
	private:	
		SceneEntity					m_SelectedEntity;
		Editor::EditorManager		m_EditorManager;


		struct GPUTimeQueries
		{
			uint32_t GPUTime = 0;

			static constexpr uint32_t Count() { return sizeof(GPUTimeQueries) / sizeof(uint32_t); }
		};
		GPUTimeQueries m_GPUTimeQueries;
	};
}