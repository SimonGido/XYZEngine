#include "InspectorPanel.h"



#include "Panel.h"

namespace XYZ {
	InspectorPanel::InspectorPanel(uint32_t id)
		:
		Panel(id)
	{
		InGui::Begin(id,"Inspector", { 0,-100 }, { 300,400 }); // To make sure it exists;
		InGui::End();
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

	bool InspectorPanel::onInspectableSelected(InspectableSelectedEvent& event)
	{
		InGui::GetWindow(m_PanelID)->Flags |= InGuiWindowFlag::Modified;
		m_Inspectable = event.GetInspectable();
		return true;
	}
	bool InspectorPanel::onInspectableDeselected(InspectableDeselectedEvent& event)
	{
		InGui::GetWindow(m_PanelID)->Flags |= InGuiWindowFlag::Modified;
		m_Inspectable = nullptr;
		return false;
	}
	void InspectorPanel::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<InspectableSelectedEvent>(Hook(&InspectorPanel::onInspectableSelected, this));
		dispatcher.Dispatch<InspectableDeselectedEvent>(Hook(&InspectorPanel::onInspectableDeselected, this));

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