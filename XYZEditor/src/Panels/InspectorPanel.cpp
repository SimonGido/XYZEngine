#include "InspectorPanel.h"

namespace XYZ {
	InspectorPanel::InspectorPanel()
	{
		InGui::Begin("Inspector", { 0,-100 }, { 300,400 }); // To make sure it exists;
		InGui::End();
	}
	void InspectorPanel::OnInGuiRender()
	{
		if (InGui::Begin("Inspector", { 0,-100 }, { 300,400 }))
		{
			if (m_Layout)
				m_Layout->OnInGuiRender();
		}
		InGui::End();
	}
}