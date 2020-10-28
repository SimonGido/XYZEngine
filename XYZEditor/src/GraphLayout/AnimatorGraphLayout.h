#pragma once
#include "../Panels/GraphPanel.h"

namespace XYZ {

	class AnimatorGraphLayout : public GraphLayout
	{	
	public:
		AnimatorGraphLayout();
		virtual void OnInGuiRender() override;
		virtual void OnUpdate(Timestep ts) override;

		virtual void OnEvent(Event& event) override;
		void SetGraph(Graph* graph);

		void SetContext(const Ref<AnimationController>& context);
	private:
		void setupGraph();
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);

		struct Node
		{
			glm::vec2 Position = { 0,0 };
		};

	private:
		static constexpr int32_t sc_InvalidIndex = -1;
		static constexpr glm::vec2 sc_NodeSize = { 200,100 };

		Ref<AnimationController> m_Context = nullptr;
		Graph* m_Graph = nullptr;

		int32_t m_SelectedNode = sc_InvalidIndex;
		int32_t m_MovedNode = sc_InvalidIndex;
		Node m_NodeMap[StateMachine::GetMaxBit()];

		glm::vec2 m_MousePosition = { 0,0 };
		glm::vec2 m_MouseMovedNodeDiff = { 0,0 };
		glm::vec2 m_ArrowStartPos = { 0,0 };
		glm::vec2 m_PopupPosition = { 0,0 };
		float m_Speed = 200.0f;

		bool m_PopupEnabled = false;
		bool m_PopupOpen = false;
	};
}