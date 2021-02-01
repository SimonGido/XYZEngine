#include "stdafx.h"
#include "ScenePanel.h"



namespace XYZ {
	ScenePanel::ScenePanel(uint32_t panelID)
		:
		m_PanelID(panelID)
	{
		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		uint32_t windowWidth = Application::Get().GetWindow().GetWidth();
		uint32_t windowHeight = Application::Get().GetWindow().GetHeight();
		m_EditorCamera.SetViewportSize((float)windowWidth, (float)windowHeight);

		InGui::ImageWindow(m_PanelID, "Scene", glm::vec2(0.0f), glm::vec2(200.0f), m_SubTexture);
		InGui::End();
	}
	void ScenePanel::SetContext(Ref<Scene> context)
	{
		m_Context = context;
	}
	void ScenePanel::SetSubTexture(Ref<SubTexture> subTexture)
	{
		m_SubTexture = subTexture;
	}

	void ScenePanel::OnUpdate(Timestep ts)
	{
		if (IS_SET(InGui::GetWindowFlags(m_PanelID), InGuiWindowFlags::Hoovered))
		{
			m_EditorCamera.OnUpdate(ts);
		}
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
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>(Hook(&ScenePanel::onWindowResize, this));
		if (IS_SET(InGui::GetWindowFlags(m_PanelID), InGuiWindowFlags::Hoovered))
		{
			m_EditorCamera.OnEvent(event);
		}
	}
	bool ScenePanel::onWindowResize(WindowResizeEvent& event)
	{
		m_EditorCamera.SetViewportSize((float)event.GetWidth(), (float)event.GetHeight());
		return false;
	}
}