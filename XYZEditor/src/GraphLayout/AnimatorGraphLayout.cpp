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
			if (m_PopupEnabled)
			{
				if (InGui::BeginPopup("Create Node", m_PopupPosition, glm::vec2{ 150,25 }, m_PopupOpen))
				{
					if (InGui::PopupItem("Empty Node", { 150,25 }))
					{
						auto state = m_Context->CreateState("Empty State " + std::to_string(++s_NextID));
						bool tmpModified = false;
						InGui::BeginNode(state.GetID(), "Empty State " + std::to_string(s_NextID), InGui::GetCurrentNodeWindow()->RelativeMousePosition, { 150, 100 }, tmpModified);
						InGui::EndNode();
						m_PopupEnabled = false;
						m_PopupOpen = false;

						m_ModifiedMap.insert({ state.GetID(),false });
					}
				}
				InGui::EndPopup();
			}

			std::pair<uint32_t, uint32_t> connection;
			if (InGui::BeginConnection(connection))
			{
				m_Graph->Connect({ connection.first,connection.second });
				m_Context->GetState(connection.first).AllowTransition(connection.second);
			}

			for (auto& state : m_Context->GetStatesMap())
			{
				uint32_t id = state.second.State.GetID();
				InGui::BeginNode(id, state.second.Name, { 0,0 }, { 150,100 }, m_ModifiedMap[id]);
				InGui::EndNode();
			}

			for (auto& connection : m_Graph->GetConnections())
			{
				InGui::PushConnection(connection.Start, connection.End);
			}

			if (InGui::GetCurrentWindow()->Flags & InGuiWindowFlag::RightClicked)
			{
				auto [width, height] = Input::GetWindowSize();
				auto [mx, my] = Input::GetMousePosition();

				m_PopupEnabled = !m_PopupEnabled;
				m_PopupPosition = MouseToWorld({ mx,my }, { width,height });
			}
		}
	}
}
