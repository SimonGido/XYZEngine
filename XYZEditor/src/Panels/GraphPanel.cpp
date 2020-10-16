#include "GraphPanel.h"

namespace XYZ {
	GraphPanel::GraphPanel()
	{
		m_FBO = FrameBuffer::Create({ 100,100,{0.5f,0.5f,0.5f,1.0f} });
		m_FBO->CreateColorAttachment(FrameBufferFormat::RGBA16F);
		m_FBO->CreateDepthAttachment();
		m_FBO->Resize();
		
		InGui::RenderWindow(m_GraphID, "Graph", m_FBO->GetColorAttachment(0).RendererID, { 0,0 }, { 100,100 });
		InGui::End();
		m_GraphWindow = InGui::GetWindow(m_GraphID);
		m_GraphWindow->OnResizeCallback = [&](const glm::vec2& size) {
			// Camera OnResize
		};

		auto [width, height] = Input::GetWindowSize();
		m_FBO->SetSpecification({ (uint32_t)width,(uint32_t)height });
		m_FBO->Resize();
	}
	bool GraphPanel::OnInGuiRender(float dt)
	{
		m_ActiveWindow = false;
		if (InGui::RenderWindow(m_GraphID, "Graph", m_FBO->GetColorAttachment(0).RendererID, { 0,0 }, { 100,100 }))
		{
			if (m_Layout)
				m_Layout->OnInGuiRender();

			m_ActiveWindow = true;
		}
		InGui::End();
		return m_ActiveWindow;
	}
	void GraphPanel::OnEvent(Event& event)
	{
		if (m_ActiveWindow)
		{
			// Camera on event
		}	

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>(Hook(&GraphPanel::onWindowResize, this));
	}
	bool GraphPanel::onWindowResize(WindowResizeEvent& event)
	{
		auto& spec = m_FBO->GetSpecification();
		spec.Width  = (uint32_t)event.GetWidth();
		spec.Height = (uint32_t)event.GetHeight();
		m_FBO->Resize();
		return false;
	}
}