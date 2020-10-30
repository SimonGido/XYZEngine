#pragma once
#include <XYZ.h>

#include "Panel.h"

namespace XYZ {

	class AnimatorPanel : public Panel,
						  public EventCaller
	{
	public:
		AnimatorPanel(uint32_t id);

		virtual void OnInGuiRender() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;

		void SetContext(const Ref<AnimationController>& context);
	private:
		bool onWindowResize(WindowResizeEvent& event);
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onKeyPress(KeyPressedEvent& event);
		
		void setupGraph();
	private:
		struct Node
		{
			glm::vec2 Position = { 0,0 };
		};
		static constexpr int32_t sc_InvalidIndex = -1;
		static constexpr glm::vec2 sc_NodeSize = { 200,100 };
		
		glm::vec2 m_MousePosition = { 0,0 };
		glm::vec2 m_MouseMovedNodeDiff = { 0,0 };
		glm::vec2 m_ArrowStartPos = { 0,0 };
		glm::vec2 m_PopupPosition = { 0,0 };
		float m_Speed = 200.0f;

		int32_t m_SelectedNode = sc_InvalidIndex;
		int32_t m_MovedNode = sc_InvalidIndex;

		bool m_PopupEnabled = false;
		bool m_PopupOpen = false;		
	private:
		Ref<FrameBuffer> m_FBO;
		Ref<RenderPass> m_RenderPass;
		Ref<AnimationController> m_Context;
		Graph m_Graph;
		Node m_NodeMap[StateMachine::GetMaxBit()];

		EditorCamera m_Camera;
		InGuiMesh m_Mesh;
		InGuiLineMesh m_LineMesh;
	};
}