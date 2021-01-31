#include "stdafx.h"
#include "ScenePanel.h"



namespace XYZ {
	ScenePanel::ScenePanel(uint32_t panelID)
		:
		m_PanelID(panelID)
	{		
	}
	void ScenePanel::SetContext(Ref<Scene> context)
	{
		m_Context = context;
	}
	void ScenePanel::SetSubTexture(Ref<SubTexture> subTexture)
	{
		m_SubTexture = subTexture;
	}
	void ScenePanel::OnInGuiRender()
	{
		if (InGui::ImageWindow(m_PanelID, "Scene", glm::vec2(0.0f), glm::vec2(200.0f), m_SubTexture))
		{

		}
		InGui::End();
	}
	void ScenePanel::OnEvent(Event& event)
	{
	}
}