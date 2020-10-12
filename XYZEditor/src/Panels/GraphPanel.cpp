#include "GraphPanel.h"

namespace XYZ {
	GraphPanel::GraphPanel()
	{
		InGui::NodeWindow(m_GraphID, "Graph", { 0,0 }, { 100,100 }, 0.0f);
		InGui::NodeWindowEnd();
		m_GraphWindow = InGui::GetNodeWindow(m_GraphID);
		m_GraphWindow->RenderWindow->OnResizeCallback = [&](const glm::vec2& size) {
			m_GraphWindow->InCamera.OnResize(size);
		};
	}
	bool GraphPanel::OnInGuiRender(float dt)
	{
		m_ActiveWindow = false;
		if (InGui::NodeWindow(m_GraphID, "Graph", { 0,0 }, { 100,100 }, dt))
		{
			if (m_Layout)
				m_Layout->OnInGuiRender();

			m_ActiveWindow = true;
		}
		InGui::NodeWindowEnd();
		return m_ActiveWindow;
	}
	void GraphPanel::OnEvent(Event& event)
	{
		if (m_ActiveWindow)
			m_GraphWindow->InCamera.OnEvent(event);

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>(Hook(&GraphPanel::onWindowResize, this));
	}
	bool GraphPanel::onWindowResize(WindowResizeEvent& event)
	{
		auto& spec = m_GraphWindow->FBO->GetSpecification();
		spec.Width  = (uint32_t)event.GetWidth();
		spec.Height = (uint32_t)event.GetHeight();
		m_GraphWindow->FBO->Resize();
		return false;
	}
}