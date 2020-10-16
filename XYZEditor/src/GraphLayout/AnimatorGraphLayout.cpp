#include "AnimatorGraphLayout.h"


namespace XYZ {
	static glm::vec2 MouseToWorld(const glm::vec2& point, const glm::vec2& windowSize)
	{
		glm::vec2 offset = { windowSize.x / 2,windowSize.y / 2 };
		return { point.x - offset.x, offset.y - point.y};
	}

	static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
	{
		return (pos.x + size.x > point.x &&
			pos.x		   < point.x&&
			pos.y + size.y >  point.y &&
			pos.y < point.y);
	}

	AnimatorGraphLayout::AnimatorGraphLayout()
		:
		m_Graph(false)
	{
	}

	void AnimatorGraphLayout::OnInGuiRender()
	{
		if (m_Context)
		{
			if (InGui::GetCurrentWindow()->Flags & InGuiWindowFlag::RightClicked)
			{
				auto [width, height] = Input::GetWindowSize();
				auto [mx, my] = Input::GetMousePosition();

				m_PopupEnabled = !m_PopupEnabled;
				m_PopupPosition = MouseToWorld({ mx,my }, { width,height });
			}

		
			auto& machine = m_Context->GetStateMachine();
			m_Graph.TraverseAll([&](int32_t source, int32_t destination, int32_t sourceIndex, int32_t destinationIndex, bool isConnected) {
				if (!isConnected)
				{
					glm::vec2 pos = m_NodeMap[source].Position + m_ScreenOffset;
					InGui::BeginNode(machine.GetStateName(sourceIndex).c_str(), pos, sc_NodeSize);
					pos += sc_NodeSize / 2.0f;
					
				}
				else
				{
					glm::vec2 sourcePos = m_NodeMap[source].Position + m_ScreenOffset;
					InGui::BeginNode(machine.GetStateName(sourceIndex).c_str(), sourcePos, sc_NodeSize);
					glm::vec2 destPos = m_NodeMap[destination].Position + m_ScreenOffset;
					InGui::BeginNode(machine.GetStateName(destinationIndex).c_str(), destPos, sc_NodeSize);
					
					sourcePos += sc_NodeSize / 2.0f;
					destPos += sc_NodeSize / 2.0f;
					InGui::PushArrow(sourcePos, destPos, { 50,50 });
				}		
			});

			if (m_SelectedNode != sc_InvalidIndex)
			{
				auto [width, height] = Input::GetWindowSize();
				auto [mx, my] = Input::GetMousePosition();
				auto mousePos = MouseToWorld({ mx,my }, { width,height });
				
				InGui::PushArrow(m_ArrowStartPos, mousePos, { 50,50 });
			}
		}
	}
	void AnimatorGraphLayout::OnUpdate(Timestep ts)
	{
		if (Input::IsKeyPressed(KeyCode::XYZ_KEY_A))
		{
			m_ScreenOffset.x += m_Speed * ts;
		}
		else if (Input::IsKeyPressed(KeyCode::XYZ_KEY_D))
		{
			m_ScreenOffset.x -= m_Speed * ts;
		}

		if (Input::IsKeyPressed(KeyCode::XYZ_KEY_W))
		{
			m_ScreenOffset.y -= m_Speed * ts;
		}
		else if (Input::IsKeyPressed(KeyCode::XYZ_KEY_S))
		{
			m_ScreenOffset.y += m_Speed * ts;
		}
	}
	void AnimatorGraphLayout::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&AnimatorGraphLayout::onMouseButtonPress, this));
		dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&AnimatorGraphLayout::onMouseButtonRelease, this));
	}
	void AnimatorGraphLayout::SetContext(const Ref<AnimationController>& context)
	{ 
		m_Context = context; 

		glm::vec2 pos = { -400,0 };
		uint32_t counter = 0;
		auto& machine = m_Context->GetStateMachine();
		m_NodeMap.resize(machine.GetStatesMap().size());
		for (auto it : machine.GetStatesMap())
		{
			int32_t index = m_Graph.AddVertex(it.second.State.GetID());
			m_NodeMap[index] = { pos };
			pos.x += 150;
			pos.y += 150;
			if (counter++ % 2)
				pos.y -= 500;
		}
		for (auto& [index, pairN] : machine.GetStatesMap())
		{
			for (auto& [index2, pairK] : machine.GetStatesMap())
			{
				auto& source = pairN.State;
				auto& destination = pairK.State;
				if (source.CanTransitTo(destination.GetID()))
				{
					m_Graph.AddEdge(source.GetID(), destination.GetID());
				}
			}
		}
	}
	bool AnimatorGraphLayout::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		if (event.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
		{
			auto [mx, my] = Input::GetMousePosition();
			auto [width, height] = Input::GetWindowSize();
			glm::vec2 mousePos = MouseToWorld({ mx,my }, { width,height });
			m_Graph.TraverseAll([&](int32_t source, int32_t destination, int32_t sourceIndex, int32_t destinationIndex, bool isConnected) {
				
				if (!isConnected)
				{
					glm::vec2 pos = m_NodeMap[source].Position + m_ScreenOffset;
					if (Collide(pos, sc_NodeSize, mousePos))
					{
						m_SelectedNode = sourceIndex;
						m_ArrowStartPos = pos + sc_NodeSize / 2.0f;
					}
				}
				else
				{
					glm::vec2 sourcePos = m_NodeMap[source].Position + m_ScreenOffset;
					glm::vec2 destPos = m_NodeMap[destination].Position + m_ScreenOffset;
					if (Collide(sourcePos, sc_NodeSize, mousePos))
					{
						m_SelectedNode = sourceIndex;
						m_ArrowStartPos = sourcePos + sc_NodeSize / 2.0f;
					}
					else if (Collide(destPos, sc_NodeSize, mousePos))
					{
						m_SelectedNode = destinationIndex;
						m_ArrowStartPos = destPos + sc_NodeSize / 2.0f;
					}
				}

			});
		}
		return false;
	}
	bool AnimatorGraphLayout::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		if (event.IsButtonReleased(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
		{
			auto [mx, my] = Input::GetMousePosition();
			auto [width, height] = Input::GetWindowSize();
			glm::vec2 mousePos = MouseToWorld({ mx,my }, { width,height });
			int32_t endIndex = sc_InvalidIndex;
			m_Graph.TraverseAll([&](int32_t source, int32_t destination, int32_t sourceIndex, int32_t destinationIndex, bool isConnected) {

				if (!isConnected)
				{
					glm::vec2 pos = m_NodeMap[source].Position + m_ScreenOffset;
					if (Collide(pos, sc_NodeSize, mousePos))
						endIndex = sourceIndex;
				}
				else
				{
					glm::vec2 sourcePos = m_NodeMap[source].Position + m_ScreenOffset;
					glm::vec2 destPos = m_NodeMap[destination].Position + m_ScreenOffset;
					if (Collide(sourcePos, sc_NodeSize, mousePos))
						endIndex = sourceIndex;
					else if (Collide(destPos, sc_NodeSize, mousePos))
						endIndex = destinationIndex;
				}
			});
			if (endIndex != sc_InvalidIndex && m_SelectedNode != sc_InvalidIndex)
			{
				m_Graph.AddEdge(m_SelectedNode, endIndex);
				m_Context->GetStateMachine().GetState(m_SelectedNode).AllowTransition(endIndex);
			}
			m_SelectedNode = sc_InvalidIndex;
		}
		return false;
	}
}
