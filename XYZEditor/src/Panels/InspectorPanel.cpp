#include "InspectorPanel.h"



#include "Panel.h"

namespace XYZ {
	InspectorPanel::InspectorPanel(uint32_t id)
		:
		Panel(id)
	{
		InGui::Begin(id,"Inspector", { 0,-100 }, { 300,400 }); // To make sure it exists;
		InGui::End();
		InGui::GetWindow(id)->Flags |= InGuiWindowFlag::MenuEnabled;
	}
	void InspectorPanel::OnInGuiRender()
	{
		if (InGui::Begin(m_PanelID, "Inspector", { 0,-100 }, { 300,400 }))
		{
			if (m_Inspectable)
				m_Inspectable->OnInGuiRender();
			
		}
		InGui::End();
	}
	void InspectorPanel::OnUpdate(Timestep ts)
	{
		if (InGui::GetWindow(m_PanelID)->Flags & InGuiWindowFlag::Hoovered)
		{
			if (m_Inspectable)
				m_Inspectable->OnUpdate(ts);
		}
	}
	void InspectorPanel::OnEvent(Event& event)
	{
		if (InGui::GetWindow(m_PanelID)->Flags & InGuiWindowFlag::Hoovered)
		{	
			if (m_Inspectable)
				m_Inspectable->OnEvent(event);
		}
	}

	void InspectorPanel::SetInspectable(Inspectable* inspectable)
	{
		m_Inspectable = inspectable;
		InGui::GetWindow(m_PanelID)->Flags |= InGuiWindowFlag::Modified;
	}
	
}