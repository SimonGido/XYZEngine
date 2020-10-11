#include "GraphPanel.h"

namespace XYZ {
	GraphPanel::GraphPanel()
	{
		InGui::NodeWindow(m_GraphID, "Graph", { 0,0 }, { 100,100 }, 0.0f);
		InGui::NodeWindowEnd();
		m_GraphWindow = InGui::GetNodeWindow(m_GraphID);
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
	}
}