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


				InGui::BeginScrollableArea(300.0f, m_ScrollOffset, 100.0f);
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
						InGui::Separator();
						InGui::Checkbox("Visible", glm::vec2(30.0f), spriteRenderer.IsVisible);
						InGui::Separator();
					}
					InGui::Separator();
				}
				if (m_Context.HasComponent<PointLight2D>())
				{
					PointLight2D& light = m_Context.GetComponent<PointLight2D>();
					if (InGui::BeginGroup("Point Light 2D", m_ComponentGroups[index++]))
					{
						InGui::Text("Color");
						InGui::Separator();
						InGui::Float("R", glm::vec2(50.0f, 30.0f), light.Color.x);
						InGui::Float("G", glm::vec2(50.0f, 30.0f), light.Color.y);
						InGui::Float("B", glm::vec2(50.0f, 30.0f), light.Color.z);
						InGui::Separator();
						InGui::Text("Intensity");
						InGui::Float("", glm::vec2(50.0f, 25.0f), light.Intensity);
						InGui::Separator();
					}
					InGui::Separator();
				}
				if (m_Context.HasComponent<ScriptComponent>())
				{
					ScriptComponent& script = m_Context.GetComponent<ScriptComponent>();
					if (InGui::BeginGroup("Script Component", m_ComponentGroups[index++]))
					{
						InGui::String("Module Name", glm::vec2(120.0f, 25.0f), script.ModuleName);
						InGui::Separator();
						InGui::Text("Public Fields:");
						InGui::Separator();
						for (auto& field : script.Fields)
						{			
							if (field.GetType() == PublicFieldType::Float)
							{
								float val = field.GetStoredValue<float>();
								if (InGui::Float(field.GetName().c_str(), glm::vec2(50.0f, 30.0f), val) == InGuiReturnType::Modified)
								{
									field.SetStoredValue<float>(val);
									field.SetRuntimeValue<float>(val);
								}
							}
							else if (field.GetType() == PublicFieldType::Vec2)
							{
								glm::vec2 val = field.GetStoredValue<glm::vec2>();
								if (InGui::Float("", glm::vec2(50.0f, 30.0f), val.x) == InGuiReturnType::Modified)
								{
									field.SetStoredValue<glm::vec2>(val);
									field.SetRuntimeValue<glm::vec2>(val);
								}
								if (InGui::Float(field.GetName().c_str(), glm::vec2(50.0f, 30.0f), val.y) == InGuiReturnType::Modified)
								{
									field.SetStoredValue<glm::vec2>(val);
									field.SetRuntimeValue<glm::vec2>(val);
								}
							}
							else if (field.GetType() == PublicFieldType::Vec3)
							{
								glm::vec3 val = field.GetStoredValue<glm::vec3>();								
								if (InGui::Float("", glm::vec2(50.0f, 30.0f), val.x) == InGuiReturnType::Modified)
								{
									field.SetStoredValue<glm::vec3>(val);
									field.SetRuntimeValue<glm::vec3>(val);
								}
								if (InGui::Float("", glm::vec2(50.0f, 30.0f), val.y) == InGuiReturnType::Modified)
								{
									field.SetStoredValue<glm::vec3>(val);
									field.SetRuntimeValue<glm::vec3>(val);
								}
								if (InGui::Float(field.GetName().c_str(), glm::vec2(50.0f, 30.0f), val.z) == InGuiReturnType::Modified)
								{
									field.SetStoredValue<glm::vec3>(val);
									field.SetRuntimeValue<glm::vec3>(val);
								}
							}
							else if (field.GetType() == PublicFieldType::Vec4)
							{
								glm::vec4 val = field.GetStoredValue<glm::vec4>();
								if (InGui::Float("", glm::vec2(50.0f, 30.0f), val.x) == InGuiReturnType::Modified)
								{
									field.SetStoredValue<glm::vec4>(val);
									field.SetRuntimeValue<glm::vec4>(val);
								}
								if (InGui::Float("", glm::vec2(50.0f, 30.0f), val.y) == InGuiReturnType::Modified)
								{
									field.SetStoredValue<glm::vec4>(val);
									field.SetRuntimeValue<glm::vec4>(val);
								}
								if (InGui::Float("", glm::vec2(50.0f, 30.0f), val.z) == InGuiReturnType::Modified)
								{
									field.SetStoredValue<glm::vec4>(val);
									field.SetRuntimeValue<glm::vec4>(val);
								}
								if (InGui::Float(field.GetName().c_str(), glm::vec2(50.0f, 30.0f), val.w) == InGuiReturnType::Modified)
								{
									field.SetStoredValue<glm::vec4>(val);
									field.SetRuntimeValue<glm::vec4>(val);
								}
							}
							else if (field.GetType() == PublicFieldType::String)
							{
								std::string val = field.GetStoredValue<std::string>();
								if (InGui::String(field.GetName().c_str(), glm::vec2(50.0f, 30.0f), val) == InGuiReturnType::Modified)
								{
									field.SetStoredValue<std::string>(val);
									field.SetRuntimeValue<std::string>(val);			
								}
							}
							InGui::Separator();
						}
					}
					InGui::Separator();
				}
				InGui::Separator();
				InGui::EndScrollableArea();
				InGui::Separator();
				if (!m_Context.HasComponent<ScriptComponent>())
				{
					InGui::Dropdown("Add Component", glm::vec2(InGui::GetWindow(m_PanelID).Size.x - 20.0f, 25.0f), m_AddComponentOpen);
				}
			}
			InGui::Separator();
			InGui::Button("test", glm::vec2(50.0f));
			InGui::Button("test", glm::vec2(50.0f));
			InGui::Button("test", glm::vec2(50.0f));
			InGui::Button("test", glm::vec2(50.0f));
			InGui::Separator();
			InGui::Button("test", glm::vec2(50.0f));
			InGui::Separator();
			InGui::Button("test", glm::vec2(50.0f));
			InGui::Separator();
			InGui::Button("test", glm::vec2(50.0f));
			InGui::Separator();
			InGui::Button("test", glm::vec2(50.0f));
			InGui::Separator();
			InGui::Button("test", glm::vec2(50.0f));
			InGui::Separator();
			InGui::Button("test", glm::vec2(50.0f));
			InGui::Button("test", glm::vec2(50.0f));
			InGui::Button("test", glm::vec2(50.0f));
			InGui::Button("test", glm::vec2(50.0f));
			InGui::Separator();
			InGui::Button("test", glm::vec2(50.0f));
			InGui::Separator();
			InGui::Button("test", glm::vec2(50.0f));
			InGui::Separator();
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