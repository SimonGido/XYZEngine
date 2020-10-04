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
		bool active = false;
		if (InGui::NodeWindow(m_GraphID, "Graph", { 0,0 }, { 100,100 }, dt))
		{
			if (m_Layout)
				m_Layout->OnInGuiRender();

			active = true;
		}
		InGui::NodeWindowEnd();
		return active;
	}
	void GraphPanel::OnEvent(Event& event)
	{
		m_GraphWindow->InCamera.OnEvent(event);
	}
}