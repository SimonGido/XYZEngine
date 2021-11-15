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

	private:
		void displayStats();
		void gpuParticleExample(SceneEntity entity);
		void cpuParticleExample(SceneEntity entity);
		void animationExample(SceneEntity entity);

	private:
		enum Editors
		{
			SpriteEditor, AnimationEditor, NumEditors
		};
		bool m_EditorOpen[NumEditors];
		
		Ref<Scene>					m_Scene;
		Ref<SceneRenderer>			m_SceneRenderer;
		Ref<Renderer2D>				m_SceneRenderer2D;
		Ref<EditorRenderer>			m_EditorRenderer;

	private:	
		SceneEntity m_SelectedEntity;


	private:
		Ref<Shader>		 m_Shader;
		Ref<RenderPass>  m_RenderPass;
		Ref<Pipeline>	 m_Pipeline;
		Ref<Framebuffer> m_Framebuffer;
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer>  m_IndexBuffer;
		Ref<RenderCommandBuffer> m_RenderCommandBuffer;
	};
}