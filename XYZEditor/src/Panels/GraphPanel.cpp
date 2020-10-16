#include "GraphPanel.h"

namespace XYZ {
	GraphPanel::GraphPanel()
	{
		InGui::Begin(m_GraphID, "Graph", { 0,0 }, { 100,100 });
		InGui::End();
		m_GraphWindow = InGui::GetWindow(m_GraphID);
		m_GraphWindow->Flags &= ~InGuiWindowFlag::AutoPosition;
		m_GraphWindow->Flags &= ~InGuiWindowFlag::EventListener;
	}
	bool GraphPanel::OnInGuiRender(Timestep ts)
	{
		m_ActiveWindow = false;

		if (InGui::Begin(m_GraphID, "Graph", { 0,0 }, { 100,100 }))
		{
			if (m_Layout)
			{
				m_Layout->OnInGuiRender();
				if (m_GraphWindow->Flags & InGuiWindowFlag::Hoovered)
					m_Layout->OnUpdate(ts);
			}
			m_ActiveWindow = true;
		}
		InGui::End();
		return m_ActiveWindow;
	}

	void GraphPanel::OnEvent(Event& event)
	{
		if (m_Layout)
			m_Layout->OnEvent(event);
	}

}