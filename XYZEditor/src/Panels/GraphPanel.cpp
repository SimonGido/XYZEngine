#include "GraphPanel.h"

#include "Panel.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace XYZ {

	GraphPanel::GraphPanel()
	{
		auto& app = Application::Get();
		m_FBO = FrameBuffer::Create({ app.GetWindow().GetWidth(),app.GetWindow().GetHeight(),{0.2f,0.2f,0.2f,1.0f} });
		m_FBO->CreateColorAttachment(FrameBufferFormat::RGBA16F);
		m_FBO->CreateDepthAttachment();
		m_FBO->Resize();
		m_RenderPass = RenderPass::Create({ m_FBO });

		InGui::RenderWindow(PanelID::GraphPanel, "Graph", m_FBO->GetColorAttachment(0).RendererID, { 0,0 }, { 100,100 });
		InGui::End();
		m_GraphWindow = InGui::GetWindow(PanelID::GraphPanel);
		m_GraphWindow->Flags &= ~InGuiWindowFlag::AutoPosition;
		m_GraphWindow->Flags &= ~InGuiWindowFlag::EventListener;
		m_GraphWindow->OnResizeCallback = [this](const glm::vec2& size) {
			m_Camera.OnResize(size);
		};
		m_Camera.SetCameraMouseMoveSpeed(0.002f);
	}
	bool GraphPanel::OnInGuiRender(Timestep ts)
	{	
		m_ActiveWindow = false;
		if (InGui::RenderWindow(PanelID::GraphPanel, "Graph",m_FBO->GetColorAttachment(0).RendererID, { 0,0 }, { 100,100 }))
		{
			m_Camera.OnUpdate(ts);
			if (m_Layout)
				m_Layout->OnInGuiRender();
			m_ActiveWindow = true;
		}
		InGui::End();

		if (m_ActiveWindow)
		{
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
			glm::vec2 mousePos = InGui::GetInGuiPosition(*m_GraphWindow, m_Camera.GetPosition(), m_Camera.GetAspectRatio(), m_Camera.GetZoomLevel());
			InGui::SetMousePosition(mousePos);
			if (m_Layout)
			{
				m_Layout->OnUpdate(ts);
			}
			
			InGui::EndSubFrame();
			Renderer::EndRenderPass();
		}
		else
		{
			m_Camera.Stop();
		}
		return m_ActiveWindow;
	}

	void GraphPanel::OnEvent(Event& event)
	{
		if (m_Layout)
			m_Layout->OnEvent(event);
		if (m_ActiveWindow)
			m_Camera.OnEvent(event);

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>(Hook(&GraphPanel::onWindowResize, this));
	}

	bool GraphPanel::onWindowResize(WindowResizeEvent& event)
	{
		auto& spec = m_FBO->GetSpecification();
		spec.Width = (uint32_t)event.GetWidth();
		spec.Height = (uint32_t)event.GetHeight();
		m_FBO->Resize();
		return false;
	}

}