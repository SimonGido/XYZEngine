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
		}
	}

	void AnimatorGraphLayout::OnUpdate(Timestep ts)
	{
		if (m_Context)
		{
			m_MousePosition = InGui::GetMousePosition();
			auto& machine = m_Context->GetStateMachine();
			m_Graph.TraverseAll([&](int32_t source, int32_t destination, int32_t sourceIndex, int32_t destinationIndex, bool isConnected) {
				
				if (!isConnected)
				{
					glm::vec2 pos = m_NodeMap[source].Position;
					InGui::BeginNode(machine.GetStateName(sourceIndex).c_str(), pos, sc_NodeSize);
					pos += sc_NodeSize / 2.0f;
				}
				else
				{
					glm::vec2 sourcePos = m_NodeMap[source].Position;
					InGui::BeginNode(machine.GetStateName(sourceIndex).c_str(), sourcePos, sc_NodeSize);
					glm::vec2 destPos = m_NodeMap[destination].Position;
					InGui::BeginNode(machine.GetStateName(destinationIndex).c_str(), destPos, sc_NodeSize);

					sourcePos += sc_NodeSize / 2.0f;
					destPos += sc_NodeSize / 2.0f;
					InGui::PushArrow(sourcePos, destPos, { 50,50 });
				}
				});

			if (m_SelectedNode != sc_InvalidIndex)
			{
				InGui::PushArrow(m_ArrowStartPos, m_MousePosition, { 50,50 });
			}
			else if (m_MovedNode != sc_InvalidIndex)
			{
				m_NodeMap[m_MovedNode].Position = m_MousePosition + m_MouseMovedNodeDiff;
			}
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
		for (uint32_t i = 0; i < machine.GetNumStates(); ++i)
		{
			if (machine.IsStateInitialized(i))
			{
				int32_t index = m_Graph.AddVertex(machine.GetState(i).GetID());
				m_NodeMap[index] = { pos };
				pos.x += 150;
				pos.y += 150;
				if (counter++ % 2)
					pos.y -= 500;
			}
		}

		for (uint32_t i = 0; i < machine.GetNumStates(); ++i)
		{
			if (machine.IsStateInitialized(i))
			{
				for (uint32_t j = 0; j < machine.GetNumStates(); ++j)
				{
					if (machine.IsStateInitialized(j))
					{
						auto& source = machine.GetState(i);
						auto& destination = machine.GetState(j);
						if (source.CanTransitTo(destination.GetID()))
						{
							m_Graph.AddEdge(source.GetID(), destination.GetID());
						}
					}
				}
			}
		}
	}
	bool AnimatorGraphLayout::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		m_Graph.TraverseAll([&](int32_t source, int32_t destination, int32_t sourceIndex, int32_t destinationIndex, bool isConnected) {

			if (!isConnected)
			{
				if (Collide(m_NodeMap[source].Position, sc_NodeSize, m_MousePosition))
				{
					if (event.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
					{
						m_SelectedNode = sourceIndex;
						m_ArrowStartPos = m_NodeMap[source].Position + sc_NodeSize / 2.0f;
					}
					else if (event.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_RIGHT))
					{
						m_MovedNode = sourceIndex;
						m_MouseMovedNodeDiff = m_NodeMap[m_MovedNode].Position - m_MousePosition;
					}
				}
			}
			else
			{
				if (event.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
				{
					if (Collide(m_NodeMap[source].Position, sc_NodeSize, m_MousePosition))
					{
						m_SelectedNode = sourceIndex;
						m_ArrowStartPos = m_NodeMap[source].Position + sc_NodeSize / 2.0f;
					}
					else if (Collide(m_NodeMap[destination].Position, sc_NodeSize, m_MousePosition))
					{
						m_SelectedNode = destinationIndex;
						m_ArrowStartPos = m_NodeMap[destination].Position + sc_NodeSize / 2.0f;
					}
				}
				else if (event.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_RIGHT))
				{
					if (Collide(m_NodeMap[source].Position, sc_NodeSize, m_MousePosition))
					{
						m_MovedNode = sourceIndex;
						m_MouseMovedNodeDiff = m_NodeMap[m_MovedNode].Position - m_MousePosition;
					}
					else if (Collide(m_NodeMap[destination].Position, sc_NodeSize, m_MousePosition))
					{
						m_MovedNode = destinationIndex;
						m_MouseMovedNodeDiff = m_NodeMap[m_MovedNode].Position - m_MousePosition;
					}
				}
			}
		});	
		return false;
	}
	bool AnimatorGraphLayout::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		if (event.IsButtonReleased(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
		{
			int32_t endIndex = sc_InvalidIndex;
			m_Graph.TraverseAll([&](int32_t source, int32_t destination, int32_t sourceIndex, int32_t destinationIndex, bool isConnected) {

				if (!isConnected)
				{
					glm::vec2 pos = m_NodeMap[source].Position;
					if (Collide(pos, sc_NodeSize, m_MousePosition))
						endIndex = sourceIndex;
				}
				else
				{
					glm::vec2 sourcePos = m_NodeMap[source].Position;
					glm::vec2 destPos = m_NodeMap[destination].Position;
					if (Collide(sourcePos, sc_NodeSize, m_MousePosition))
						endIndex = sourceIndex;
					else if (Collide(destPos, sc_NodeSize, m_MousePosition))
						endIndex = destinationIndex;
				}
			});
			if (endIndex != sc_InvalidIndex && m_SelectedNode != sc_InvalidIndex && endIndex != m_SelectedNode)
			{
				m_Graph.AddEdge(m_SelectedNode, endIndex);
				m_Context->GetStateMachine().GetState(m_SelectedNode).AllowTransition(endIndex);
			}
			m_SelectedNode = sc_InvalidIndex;
		}
		else if (event.IsButtonReleased(MouseCode::XYZ_MOUSE_BUTTON_RIGHT))
		{
			m_MovedNode = sc_InvalidIndex;
		}
		return false;
	}
}
