#include "AnimatorPanel.h"

#include "../Event/EditorEvent.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace XYZ {

	static int32_t s_NextIndex = 0;

	static glm::vec2 MouseToWorld(const glm::vec2& point, const glm::vec2& windowSize)
	{
		glm::vec2 offset = { windowSize.x / 2,windowSize.y / 2 };
		return { point.x - offset.x, offset.y - point.y };
	}

	static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
	{
		return (pos.x + size.x > point.x &&
			pos.x		   < point.x&&
			pos.y + size.y >  point.y &&
			pos.y < point.y);
	}

	AnimatorPanel::AnimatorPanel(uint32_t id)
		:
		Panel(id),
		m_Graph(false)
	{
		auto& app = Application::Get();
		m_FBO = FrameBuffer::Create({ app.GetWindow().GetWidth(),app.GetWindow().GetHeight(),{0.2f,0.2f,0.2f,1.0f} });
		m_FBO->CreateColorAttachment(FrameBufferFormat::RGBA16F);
		m_FBO->CreateDepthAttachment();
		m_FBO->Resize();
		m_RenderPass = RenderPass::Create({ m_FBO });

		InGui::RenderWindow(m_PanelID, "Animator", m_FBO->GetColorAttachment(0).RendererID, { 0,0 }, { 100,100 });
		InGui::End();
		InGui::GetWindow(m_PanelID)->Flags &= ~InGuiWindowFlag::AutoPosition;
		InGui::GetWindow(m_PanelID)->Flags &= ~InGuiWindowFlag::EventBlocking;

		InGui::GetWindow(m_PanelID)->OnResizeCallback = [this](const glm::vec2& size) {
			m_Camera.OnResize(size);
		};
		m_Camera.SetCameraMouseMoveSpeed(0.002f);
	}
	
	void AnimatorPanel::OnUpdate(Timestep ts)
	{
		m_Camera.OnUpdate(ts);
		m_Mesh.Vertices.clear();
		m_LineMesh.Vertices.clear();
		Renderer::BeginRenderPass(m_RenderPass, true);
		Renderer2D::BeginScene({ m_Camera.GetViewProjectionMatrix() });
		float cameraWidth = m_Camera.GetZoomLevel() * m_Camera.GetAspectRatio() * 2;
		float cameraHeight = m_Camera.GetZoomLevel() * 2;
		glm::mat4 gridTransform = glm::translate(glm::mat4(1.0f), m_Camera.GetPosition()) * glm::scale(glm::mat4(1.0f), { cameraWidth,cameraHeight,1.0f });
		Renderer2D::SubmitGrid(gridTransform, glm::vec2(16.025f * m_Camera.GetAspectRatio(), 16.025f), 0.025f);
		Renderer2D::Flush();
		Renderer2D::FlushLines();
		Renderer2D::EndScene();

		InGui::BeginSubFrame();
		InGui::SetViewProjection(m_Camera.GetViewProjectionMatrix());
		InGui::SetInGuiMesh(&m_Mesh, &m_LineMesh);
		glm::vec2 mousePos = InGui::GetInGuiPosition(*InGui::GetWindow(m_PanelID), m_Camera.GetPosition(), m_Camera.GetAspectRatio(), m_Camera.GetZoomLevel());
		InGui::SetMousePosition(mousePos);
		
		////////////////////////////////////////////////
		m_MousePosition = InGui::GetMousePosition();
		auto& machine = m_Context->GetStateMachine();
		//m_Graph.TraverseAll([&](int32_t source, int32_t destination, int32_t sourceIndex, int32_t destinationIndex, bool isConnected) {
		//
		//	if (!isConnected)
		//	{
		//		glm::vec2 pos = m_NodeMap[source].Position;
		//		InGui::BeginNode(machine.GetStateName(sourceIndex).c_str(), pos, sc_NodeSize);
		//		pos += sc_NodeSize / 2.0f;
		//	}
		//	else
		//	{
		//		glm::vec2 sourcePos = m_NodeMap[source].Position;
		//		InGui::BeginNode(machine.GetStateName(sourceIndex).c_str(), sourcePos, sc_NodeSize);
		//		glm::vec2 destPos = m_NodeMap[destination].Position;
		//		InGui::BeginNode(machine.GetStateName(destinationIndex).c_str(), destPos, sc_NodeSize);
		//
		//		sourcePos += sc_NodeSize / 2.0f;
		//		destPos += sc_NodeSize / 2.0f;
		//		InGui::PushArrow(sourcePos, destPos, { 50,50 });
		//	}
		//});

		auto& vertices = m_Graph.GetVertices();
		uint32_t counter = 0;
		for (auto& vertex : vertices)
		{
			glm::vec2 pos = m_NodeMap[counter].Position;
			InGui::BeginNode(machine.GetStateName(counter).c_str(), pos, sc_NodeSize);
			pos += sc_NodeSize / 2.0f;
			for (auto connection : vertex.Connections)
			{
				glm::vec2 destPos = m_NodeMap[connection].Position;
				InGui::BeginNode(machine.GetStateName(connection).c_str(), destPos, sc_NodeSize);

				destPos += sc_NodeSize / 2.0f;
				InGui::PushArrow(pos, destPos, { 50,50 });
			}
			counter++;
		}
		


		if (m_SelectedNode != sc_InvalidIndex)
		{
			InGui::PushArrow(m_ArrowStartPos, m_MousePosition, { 50,50 });
		}
		else if (m_MovedNode != sc_InvalidIndex)
		{
			m_NodeMap[m_MovedNode].Position = m_MousePosition + m_MouseMovedNodeDiff;
		}
		////////////////////////////////////////
		InGui::EndSubFrame();
		Renderer::EndRenderPass();
	}

	void AnimatorPanel::OnInGuiRender()
	{	
		if (InGui::RenderWindow(m_PanelID, "Animator", m_FBO->GetColorAttachment(0).RendererID, { 0,0 }, { 100,100 }))
		{		
			if (m_PopupEnabled)
			{
				InGui::BeginPopup("Create Node", m_PopupPosition, glm::vec2{ 150,25 }, m_PopupOpen);
				if (InGui::PopupItem("Empty Node") & InGuiReturnType::Clicked)
				{
					auto state = m_Context->GetStateMachine().CreateState("Empty State " + std::to_string(s_NextIndex++));
					m_Graph.AddVertex(state.GetID());
					m_NodeMap[state.GetID()].Position = { 0,0 };

					m_PopupEnabled = false;
					m_PopupOpen = false;
				}
				InGui::EndPopup();
			}
		}
		else
		{
			m_Camera.Stop();
		}
		InGui::End();
	}

	void AnimatorPanel::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>(Hook(&AnimatorPanel::onWindowResize, this));	
		if (InGui::GetWindow(m_PanelID)->Flags & InGuiWindowFlag::Hoovered)
		{
			dispatcher.Dispatch<KeyPressedEvent>(Hook(&AnimatorPanel::onKeyPress, this));
			dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&AnimatorPanel::onMouseButtonPress, this));
			dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&AnimatorPanel::onMouseButtonRelease, this));
			m_Camera.OnEvent(event);
		}
	}

	void AnimatorPanel::SetContext(const Ref<AnimationController>& context)
	{
		m_Context = context;
		setupGraph();
	}

	bool AnimatorPanel::onWindowResize(WindowResizeEvent& event)
	{
		auto& spec = m_FBO->GetSpecification();
		spec.Width = (uint32_t)event.GetWidth();
		spec.Height = (uint32_t)event.GetHeight();
		m_FBO->Resize();
		return false;
	}

	bool AnimatorPanel::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		auto& vertices = m_Graph.GetVertices();
		if (event.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
		{
			uint32_t counter = 0;
			for (auto& vertex : vertices)
			{
				glm::vec2 pos = m_NodeMap[counter].Position;
				if (Collide(pos, sc_NodeSize, m_MousePosition))
				{
					m_SelectedNode = counter;
					m_ArrowStartPos = m_NodeMap[counter].Position + sc_NodeSize / 2.0f;
					m_Context->GetStateMachine().SetCurrentState(counter);
					Execute(AnimatorSelectedEvent(m_Context));
					return true;
				}
				for (auto connection : vertex.Connections)
				{
					glm::vec2 connPos = m_NodeMap[connection].Position;
					if (Collide(connPos, sc_NodeSize, m_MousePosition))
					{
						m_SelectedNode = connection;
						m_ArrowStartPos = m_NodeMap[connection].Position + sc_NodeSize / 2.0f;
						m_Context->GetStateMachine().SetCurrentState(connection);
						Execute(AnimatorSelectedEvent(m_Context));
						return true;
					}
				}
				counter++;
			}
		}
		else if (event.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_RIGHT))
		{
			uint32_t counter = 0;
			for (auto& vertex : vertices)
			{
				glm::vec2 pos = m_NodeMap[counter].Position;
				if (Collide(pos, sc_NodeSize, m_MousePosition))
				{
					m_MovedNode = counter;
					m_MouseMovedNodeDiff = m_NodeMap[m_MovedNode].Position - m_MousePosition;
					return true;
				}
				for (auto connection : vertex.Connections)
				{
					glm::vec2 connPos = m_NodeMap[connection].Position;
					if (Collide(connPos, sc_NodeSize, m_MousePosition))
					{
						m_MovedNode = connection;
						m_MouseMovedNodeDiff = m_NodeMap[m_MovedNode].Position - m_MousePosition;
						return true;
					}
				}
				counter++;
			}
		}
		if (event.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_RIGHT))
		{
			if (m_MovedNode == sc_InvalidIndex && m_SelectedNode == sc_InvalidIndex)
			{
				auto [width, height] = Input::GetWindowSize();
				auto [mx, my] = Input::GetMousePosition();

				m_PopupEnabled = !m_PopupEnabled;
				m_PopupPosition = MouseToWorld({ mx,my }, { width,height });
				return true;
			}
		}
		return false;
	}

	bool AnimatorPanel::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		if (event.IsButtonReleased(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
		{
			int32_t endIndex = sc_InvalidIndex;

			uint32_t counter = 0;
			auto& vertices = m_Graph.GetVertices();
			for (auto& vertex : vertices)
			{
				glm::vec2 pos = m_NodeMap[counter].Position;
				if (Collide(pos, sc_NodeSize, m_MousePosition))
				{
					endIndex = counter;
					break;
				}

				for (auto connection : vertex.Connections)
				{
					int32_t index = vertices[connection].Index;
					glm::vec2 destPos = m_NodeMap[index].Position;
					if (Collide(destPos, sc_NodeSize, m_MousePosition))
					{
						endIndex = index;
						break;
					}
				}
				counter++;
			}

			if (endIndex != sc_InvalidIndex && m_SelectedNode != sc_InvalidIndex && endIndex != m_SelectedNode)
			{
				m_Graph.AddEdge(m_SelectedNode, endIndex);
				m_Context->GetStateMachine().GetState(m_SelectedNode).AllowTransition(endIndex);
				m_SelectedNode = sc_InvalidIndex;
				return true;
			}
			m_SelectedNode = sc_InvalidIndex;
		}
		else if (event.IsButtonReleased(MouseCode::XYZ_MOUSE_BUTTON_RIGHT))
		{
			m_MovedNode = sc_InvalidIndex;
		}
		return false;
	}

	bool AnimatorPanel::onKeyPress(KeyPressedEvent& event)
	{
		return false;
	}

	void AnimatorPanel::setupGraph()
	{
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
}