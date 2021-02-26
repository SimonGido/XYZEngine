#include "stdafx.h"
#include "InspectorPanel.h"

#include "XYZ/InGui/InGui.h"

namespace XYZ {
	static const char* BodyTypeToString(RigidBody2DComponent::BodyType type)
	{
		switch (type)
		{
		case RigidBody2DComponent::BodyType::Static:
			return "Static";
		case RigidBody2DComponent::BodyType::Dynamic:
			return "Dynamic";
		case RigidBody2DComponent::BodyType::Kinematic:
			return "Kinematic";
		}
	}
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
			m_AddComponentOpen = false;
			m_RigidBodyTypeOpen = false;
			if (m_CurrentSize < m_Context.NumberOfTypes())
			{
				resizeGroups();
			}
			memset(m_ComponentGroups, 0, m_CurrentSize);
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


				const InGuiWindow& window = InGui::GetWindow(m_PanelID);
				auto layout = window.Layout;
			
				InGui::BeginScrollableArea(glm::vec2(InGui::GetWindow(m_PanelID).Size.x - 65.0f, 500.0f), m_ScrollOffset, m_ScrollScale, 10.0f);
				
				layout.LeftPadding = 20.0f;
				layout.RightPadding = 70.0f;
				InGui::SetLayout(m_PanelID, layout);
				
				glm::vec2 size = {
					window.Size.x - window.Layout.RightPadding - window.Layout.LeftPadding,
					InGuiWindow::PanelHeight
				};

				uint32_t index = 0;
				if (m_Context.HasComponent<TransformComponent>())
				{
					TransformComponent& transform = m_Context.GetComponent<TransformComponent>();
					if (InGui::BeginGroup("Transform Component", size, m_ComponentGroups[index++]))
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
					if (InGui::BeginGroup("Sprite Renderer", size, m_ComponentGroups[index++]))
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
					if (InGui::BeginGroup("Point Light 2D", size, m_ComponentGroups[index++]))
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
				if (m_Context.HasComponent<RigidBody2DComponent>())
				{
					RigidBody2DComponent& rigidBody = m_Context.GetComponent<RigidBody2DComponent>();
					if (InGui::BeginGroup("RigidBody2D", size, m_ComponentGroups[index++]))
					{
						InGui::Dropdown(BodyTypeToString(rigidBody.Type), glm::vec2(100.0f, 25.0f), m_RigidBodyTypeOpen);
						if (m_RigidBodyTypeOpen)
						{
							if (rigidBody.Type != RigidBody2DComponent::BodyType::Static)
							{
								if (IS_SET(InGui::DropdownItem("Static"), InGuiReturnType::Clicked))
								{
									rigidBody.Type = RigidBody2DComponent::BodyType::Static;
									m_RigidBodyTypeOpen = false;
								}
							}
							if (rigidBody.Type != RigidBody2DComponent::BodyType::Dynamic)
							{
								if (IS_SET(InGui::DropdownItem("Dynamic"), InGuiReturnType::Clicked))
								{
									rigidBody.Type = RigidBody2DComponent::BodyType::Dynamic;
									m_RigidBodyTypeOpen = false;
								}
							}
							if (rigidBody.Type != RigidBody2DComponent::BodyType::Kinematic)
							{
								if (IS_SET(InGui::DropdownItem("Kinematic"), InGuiReturnType::Clicked))
								{
									rigidBody.Type = RigidBody2DComponent::BodyType::Kinematic;
									m_RigidBodyTypeOpen = false;
								}
							}	
						}
						InGui::EndDropdown();
					}
					InGui::Separator();
				}
				if (m_Context.HasComponent<BoxCollider2DComponent>())
				{
					BoxCollider2DComponent& boxCollider = m_Context.GetComponent<BoxCollider2DComponent>();
					if (InGui::BeginGroup("BoxCollider2D", size, m_ComponentGroups[index++]))
					{
						InGui::Text("Offset");
						InGui::Separator();
						InGui::Float("X", glm::vec2(50.0f, 30.0f), boxCollider.Offset.x);
						InGui::Float("Y", glm::vec2(50.0f, 30.0f), boxCollider.Offset.y);
						InGui::Separator();

						InGui::Text("Size");
						InGui::Separator();
						InGui::Float("X", glm::vec2(50.0f, 30.0f), boxCollider.Size.x);
						InGui::Float("Y", glm::vec2(50.0f, 30.0f), boxCollider.Size.y);
						InGui::Separator();

						InGui::Text("Density");
						InGui::Separator();
						InGui::Float("", glm::vec2(50.0f, 30.0f), boxCollider.Density);
						InGui::Separator();
					}
					InGui::Separator();
				}
				
				if (m_Context.HasComponent<ScriptComponent>())
				{
					ScriptComponent& script = m_Context.GetComponent<ScriptComponent>();
					if (InGui::BeginGroup("Script Component", size, m_ComponentGroups[index++]))
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
				m_ScrollScale = InGui::GetPositionOfNext().y - InGui::GetWindow(m_PanelID).Position.y + m_ScrollOffset;
				InGui::Separator();
				InGui::EndScrollableArea();

				layout.LeftPadding = 60.0f;
				layout.RightPadding = 60.0f;
				InGui::SetLayout(m_PanelID, layout);
				InGui::Separator();
				InGui::SetTextCenter(InGuiTextCenter::Middle);
				InGui::Dropdown("Add Component", glm::vec2(InGui::GetWindow(m_PanelID).Size.x - 120.0f, 25.0f), m_AddComponentOpen);
				if (m_AddComponentOpen)
				{
					if (!m_Context.HasComponent<ScriptComponent>())
					{
						if (IS_SET(InGui::DropdownItem("Add Script Component"), InGuiReturnType::Clicked))
							m_AddComponentOpen = false;
					}
					if (!m_Context.HasComponent<RigidBody2DComponent>())
					{
						if (IS_SET(InGui::DropdownItem("Add RigidBody2D"), InGuiReturnType::Clicked))
							m_AddComponentOpen = false;
					}
					if (!m_Context.HasComponent<BoxCollider2DComponent>())
					{
						if (IS_SET(InGui::DropdownItem("Add BoxCollider2D"), InGuiReturnType::Clicked))
							m_AddComponentOpen = false;
					}
				}
				InGui::EndDropdown();
				
				layout.LeftPadding = 10.0f;
				layout.RightPadding = 10.0f;
				InGui::SetLayout(m_PanelID, layout);
				InGui::SetTextCenter(InGuiTextCenter::Left);
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