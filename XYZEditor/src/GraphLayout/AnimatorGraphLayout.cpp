#include "AnimatorGraphLayout.h"


namespace XYZ {
	static glm::vec2 MouseToWorld(const glm::vec2& point, const glm::vec2& windowSize)
	{
		glm::vec2 offset = { windowSize.x / 2,windowSize.y / 2 };
		return { point.x - offset.x, offset.y - point.y};
	}
	static uint32_t s_NextID = 0;

	void AnimatorGraphLayout::OnInGuiRender()
	{
		if (m_Context && m_Graph)
		{		
			if (InGui::GetCurrentWindow()->Flags & InGuiWindowFlag::RightClicked)
			{
				auto [width, height] = Input::GetWindowSize();
				auto [mx, my] = Input::GetMousePosition();

				m_PopupEnabled = !m_PopupEnabled;
				m_PopupPosition = MouseToWorld({ mx,my }, { width,height });
			}
		}
	}
	void AnimatorGraphLayout::SetContext(StateMachine* context, Graph* graph)
	{ 
		m_Context = context; 
		m_Graph = graph;
	}
}
