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
		void SetContext(const Ref<AnimationController>& context);
	private:
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);

		struct Node
		{
			glm::vec2 Position;
		};

	private:
		static constexpr int32_t sc_InvalidIndex = -1;
		static constexpr glm::vec2 sc_NodeSize = { 100,100 };

		Ref<AnimationController> m_Context = nullptr;
		Graph m_Graph;

		int32_t m_SelectedNode = sc_InvalidIndex;
		std::vector<Node> m_NodeMap;

		glm::vec2 m_ArrowStartPos = { 0,0 };
		glm::vec2 m_ScreenOffset = { 0,0 };
		glm::vec2 m_PopupPosition = { 0,0 };
		float m_Speed = 200.0f;

		bool m_PopupEnabled = false;
		bool m_PopupOpen = false;
	};
}