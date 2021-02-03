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
	void InspectorPanel::SetContext(SceneEntity context)
	{
		m_Context = context;
		if (m_Context)
		{
			if (m_CurrentSize < m_Context.NumberOfTypes())
			{
				resizeGroups();
			}
		}
	}
	void InspectorPanel::OnInGuiRender()
	{
		if (InGui::Begin(m_PanelID, "Inspector", glm::vec2(0.0f), glm::vec2(200.0f)))
		{
			if (m_Context)
			{
				if (m_CurrentSize < m_Context.NumberOfTypes())
					resizeGroups();

				uint32_t index = 0;
				if (m_Context.HasComponent<TransformComponent>())
				{
					TransformComponent& transform = m_Context.GetComponent<TransformComponent>();
					if (InGui::BeginGroup("Transform Component", m_ComponentGroups[index++]))
					{
						InGui::Text("Translation");
						InGui::Separator();
						InGui::Float("X", glm::vec2(50.0f, 30.0f), transform.Translation.x);
						InGui::Float("Y", glm::vec2(50.0f, 30.0f), transform.Translation.y);
						InGui::Float("Z", glm::vec2(50.0f, 30.0f), transform.Translation.z);
						InGui::Separator();

						InGui::Text("Rotation");
						InGui::Separator();
						InGui::Float("X", glm::vec2(50.0f, 30.0f), transform.Rotation.x);
						InGui::Float("Y", glm::vec2(50.0f, 30.0f), transform.Rotation.y);
						InGui::Float("Z", glm::vec2(50.0f, 30.0f), transform.Rotation.z);
						InGui::Separator();

						InGui::Text("Scale");
						InGui::Separator();
						InGui::Float("X", glm::vec2(50.0f, 30.0f), transform.Scale.x);
						InGui::Float("Y", glm::vec2(50.0f, 30.0f), transform.Scale.y);
						InGui::Float("Z", glm::vec2(50.0f, 30.0f), transform.Scale.z);
						InGui::Separator();
					}
					InGui::Separator();
				}
				if (m_Context.HasComponent<SpriteRenderer>())
				{
					SpriteRenderer& spriteRenderer = m_Context.GetComponent<SpriteRenderer>();
					if (InGui::BeginGroup("Sprite Renderer", m_ComponentGroups[index++]))
					{
						InGui::Text("Color");
						InGui::Separator();
						InGui::Float("R", glm::vec2(50.0f, 30.0f), spriteRenderer.Color.x);
						InGui::Float("G", glm::vec2(50.0f, 30.0f), spriteRenderer.Color.y);
						InGui::Float("B", glm::vec2(50.0f, 30.0f), spriteRenderer.Color.z);
						InGui::Float("A", glm::vec2(50.0f, 30.0f), spriteRenderer.Color.w);
						InGui::Separator();
						InGui::Text("Sort Layer");
						InGui::Separator();
						InGui::UInt("", glm::vec2(50.0f, 30.0f), spriteRenderer.SortLayer);
						std::cout << spriteRenderer.SortLayer << std::endl;
					}
					InGui::Separator();
				}
			}
		}
		InGui::End();
	}
	void InspectorPanel::resizeGroups()
	{
		m_CurrentSize = m_Context.NumberOfTypes();
		if (m_ComponentGroups)
		{
			delete[]m_ComponentGroups;
		}
		m_ComponentGroups = new bool[m_CurrentSize];
		memset(m_ComponentGroups, 0, m_CurrentSize);
	}
}