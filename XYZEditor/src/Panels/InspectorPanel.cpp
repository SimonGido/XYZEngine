#include "stdafx.h"
#include "InspectorPanel.h"

namespace XYZ {
	InspectorPanel::InspectorPanel(uint32_t panelID)
		:
		m_PanelID(panelID)
	{
		InGui::Begin(panelID, "Inspector", glm::vec2(0.0f), glm::vec2(200.0f));
		InGui::End();
	}
	void InspectorPanel::OnInGuiRender()
	{
		if (InGui::Begin(m_PanelID, "Inspector", glm::vec2(0.0f), glm::vec2(200.0f)))
		{

		}
		InGui::End();
	}
}