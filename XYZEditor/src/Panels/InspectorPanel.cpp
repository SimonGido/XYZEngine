#include "InspectorPanel.h"

namespace XYZ {
	InspectorPanel::InspectorPanel()
	{
		InGui::Begin(m_InspectorID,"Inspector", { 0,-100 }, { 300,400 }); // To make sure it exists;
		InGui::End();
	}
	bool InspectorPanel::OnInGuiRender()
	{
		bool active = false;
		if (InGui::Begin(m_InspectorID, "Inspector", { 0,-100 }, { 300,400 }))
		{
			if (m_Layout)
				m_Layout->OnInGuiRender();
			
			active = true;
		}
		InGui::End();
		return active;
	}
	void InspectorPanel::SetInspectorLayout(InspectorLayout* layout)
	{
		m_Layout = layout;
		InGui::GetWindow(m_InspectorID)->Flags |= InGuiWindowFlag::Modified;
	}
}