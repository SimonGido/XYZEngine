#include "stdafx.h"
#include "InspectorPanel.h"



namespace XYZ {
	namespace Helper {

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
		static uint32_t FindTextLength(const char* source, Ref<Font> font)
		{
			if (!source)
				return 0;

			float xCursor = 0.0f;
			uint32_t counter = 0;
			while (source[counter] != '\0')
			{
				auto& character = font->GetCharacter(source[counter]);
				xCursor += character.XAdvance;
				counter++;
			}
			return xCursor;
		}
	}
	InspectorPanel::InspectorPanel()
	{
		std::vector<IGHierarchyElement> types;
		types.push_back({ IGElementType::Window, {} });
		types[0].Children.push_back({ IGElementType::Scrollbox, {} });

		size_t handleOffset = 2;
		m_HandleStart[TransformComponent] = handleOffset;
		handleOffset += transformComponentUI(types[0].Children[0]);
		m_HandleStart[SpriteRenderer] = handleOffset;
		handleOffset += spriteRendererUI(types[0].Children[0]);

		auto [poolHandle, handleCount] = IG::AllocateUI(types, &m_Handles);
		m_PoolHandle = poolHandle;
		m_HandleCount = handleCount;

		setupTransformComponentUI();
		setupSpriteRendererUI();
	}
	void InspectorPanel::SetContext(SceneEntity context)
	{
		m_Context = context;
		if (m_Context)
		{
			prepareTransformComponentUI();
			prepareSpriteRendererUI();
		}
	}
	void InspectorPanel::OnUpdate()
	{
		if (m_Context)
		{
			updateTransformComponentUI();
			updateSpriteRendererUI();
		}
	}
	void InspectorPanel::updateTransformComponentUI()
	{		
		XYZ::TransformComponent& transform = m_Context.GetComponent<XYZ::TransformComponent>();

		uint32_t handle = m_HandleStart[TransformComponent];

		IGFloat& translationX = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 1]);
		IGFloat& translationY = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 2]);
		IGFloat& translationZ = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 3]);
		
		translationX.SetValue(transform.Translation.x);
		transform.Translation.x = translationX.GetValue();
		
		translationY.SetValue(transform.Translation.y);
		transform.Translation.y = translationY.GetValue();
		
		translationZ.SetValue(transform.Translation.z);
		transform.Translation.z = translationZ.GetValue();

		IGFloat& rotationX = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 6]);
		IGFloat& rotationY = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 7]);
		IGFloat& rotationZ = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 8]);

		rotationX.SetValue(transform.Rotation.x);
		transform.Rotation.x = rotationX.GetValue();

		rotationY.SetValue(transform.Rotation.y);
		transform.Rotation.y = rotationY.GetValue();

		rotationZ.SetValue(transform.Rotation.z);
		transform.Rotation.z = rotationZ.GetValue();

		IGFloat& scaleX = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 11]);
		IGFloat& scaleY = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 12]);
		IGFloat& scaleZ = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 13]);

		scaleX.SetValue(transform.Scale.x);
		transform.Scale.x = scaleX.GetValue();

		scaleY.SetValue(transform.Scale.y);
		transform.Scale.y = scaleY.GetValue();

		scaleZ.SetValue(transform.Scale.z);
		transform.Scale.z = scaleZ.GetValue();
	}
	void InspectorPanel::updateSpriteRendererUI()
	{
		if (m_Context.HasComponent<XYZ::SpriteRenderer>())
		{
			XYZ::SpriteRenderer& spriteRenderer = m_Context.GetComponent<XYZ::SpriteRenderer>();
			uint32_t handle = m_HandleStart[SpriteRenderer];
			IGFloat& colorR = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 1]);
			IGFloat& colorG = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 2]);
			IGFloat& colorB = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 3]);
			IGFloat& colorA = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 4]);

			IGInt& sortLayer = IG::GetUI<IGInt>(m_PoolHandle, m_Handles[handle + 6]);
			IGCheckbox& visible = IG::GetUI<IGCheckbox>(m_PoolHandle, m_Handles[handle + 8]);
			
			colorR.SetValue(spriteRenderer.Color.r);
			spriteRenderer.Color.r = colorR.GetValue();

			colorG.SetValue(spriteRenderer.Color.g);
			spriteRenderer.Color.g = colorR.GetValue();

			colorB.SetValue(spriteRenderer.Color.b);
			spriteRenderer.Color.b = colorR.GetValue();

			colorA.SetValue(spriteRenderer.Color.a);
			spriteRenderer.Color.a = colorR.GetValue();
			
			sortLayer.SetValue(spriteRenderer.SortLayer);
			spriteRenderer.SortLayer = sortLayer.GetValue();

			spriteRenderer.IsVisible = visible.Checked;
		}
	}
	//void InspectorPanel::OnInGuiRender()
	//{
		//if (InGui::Begin(m_PanelID, "Inspector", glm::vec2(0.0f), glm::vec2(200.0f)))
		//{
		//	if (m_Context)
		//	{
		//		if (m_CurrentSize < m_Context.NumberOfTypes())
		//			resizeGroups();
		//
		//
		//		const InGuiWindow& window = InGui::GetWindow(m_PanelID);
		//		auto layout = window.Layout;
		//	
		//		InGui::BeginScrollableArea(glm::vec2(InGui::GetWindow(m_PanelID).Size.x - 20.0f, 500.0f), m_ScrollOffset, m_ScrollScale, 10.0f);
		//		
		//		layout.LeftPadding = 20.0f;
		//		layout.RightPadding = 70.0f;
		//		InGui::SetLayout(m_PanelID, layout);
		//		
		//		glm::vec2 size = {
		//			window.Size.x - window.Layout.RightPadding - window.Layout.LeftPadding,
		//			InGuiWindow::PanelHeight
		//		};
		//
		//		uint32_t index = 0;
		//		if (m_Context.HasComponent<TransformComponent>())
		//		{
		//			TransformComponent& transform = m_Context.GetComponent<TransformComponent>();
		//			if (InGui::BeginGroup("Transform Component", size, m_ComponentGroups[index++]))
		//			{
		//				InGui::Text("Translation");
		//				InGui::Separator();
		//				InGui::Float("X", glm::vec2(50.0f, 30.0f), transform.Translation.x);
		//				InGui::Float("Y", glm::vec2(50.0f, 30.0f), transform.Translation.y);
		//				InGui::Float("Z", glm::vec2(50.0f, 30.0f), transform.Translation.z);
		//				InGui::Separator();
		//
		//				InGui::Text("Rotation");
		//				InGui::Separator();
		//				InGui::Float("X", glm::vec2(50.0f, 30.0f), transform.Rotation.x);
		//				InGui::Float("Y", glm::vec2(50.0f, 30.0f), transform.Rotation.y);
		//				InGui::Float("Z", glm::vec2(50.0f, 30.0f), transform.Rotation.z);
		//				InGui::Separator();
		//
		//				InGui::Text("Scale");
		//				InGui::Separator();
		//				InGui::Float("X", glm::vec2(50.0f, 30.0f), transform.Scale.x);
		//				InGui::Float("Y", glm::vec2(50.0f, 30.0f), transform.Scale.y);
		//				InGui::Float("Z", glm::vec2(50.0f, 30.0f), transform.Scale.z);
		//				InGui::Separator();
		//			}
		//			InGui::Separator();
		//		}
		//		if (m_Context.HasComponent<SpriteRenderer>())
		//		{
		//			SpriteRenderer& spriteRenderer = m_Context.GetComponent<SpriteRenderer>();
		//			if (InGui::BeginGroup("Sprite Renderer", size, m_ComponentGroups[index++]))
		//			{
		//				InGui::Text("Color");
		//				InGui::Separator();
		//				InGui::Float("R", glm::vec2(50.0f, 30.0f), spriteRenderer.Color.x);
		//				InGui::Float("G", glm::vec2(50.0f, 30.0f), spriteRenderer.Color.y);
		//				InGui::Float("B", glm::vec2(50.0f, 30.0f), spriteRenderer.Color.z);
		//				InGui::Float("A", glm::vec2(50.0f, 30.0f), spriteRenderer.Color.w);
		//				InGui::Separator();
		//				InGui::Text("Sort Layer");
		//				InGui::Separator();
		//				InGui::UInt("", glm::vec2(50.0f, 30.0f), spriteRenderer.SortLayer);
		//				InGui::Separator();
		//				InGui::Checkbox("Visible", glm::vec2(30.0f), spriteRenderer.IsVisible);
		//				InGui::Separator();
		//			}
		//			InGui::Separator();
		//		}
		//		if (m_Context.HasComponent<PointLight2D>())
		//		{
		//			PointLight2D& light = m_Context.GetComponent<PointLight2D>();
		//			if (InGui::BeginGroup("Point Light 2D", size, m_ComponentGroups[index++]))
		//			{
		//				InGui::Text("Color");
		//				InGui::Separator();
		//				InGui::Float("R", glm::vec2(50.0f, 30.0f), light.Color.x);
		//				InGui::Float("G", glm::vec2(50.0f, 30.0f), light.Color.y);
		//				InGui::Float("B", glm::vec2(50.0f, 30.0f), light.Color.z);
		//				InGui::Separator();
		//				InGui::Text("Intensity");
		//				InGui::Float("", glm::vec2(50.0f, 25.0f), light.Intensity);
		//				InGui::Separator();
		//			}
		//			InGui::Separator();
		//		}
		//		if (m_Context.HasComponent<RigidBody2DComponent>())
		//		{
		//			RigidBody2DComponent& rigidBody = m_Context.GetComponent<RigidBody2DComponent>();
		//			if (InGui::BeginGroup("RigidBody2D", size, m_ComponentGroups[index++]))
		//			{
		//				InGui::Dropdown(BodyTypeToString(rigidBody.Type), glm::vec2(100.0f, 25.0f), m_RigidBodyTypeOpen);
		//				if (m_RigidBodyTypeOpen)
		//				{
		//					if (rigidBody.Type != RigidBody2DComponent::BodyType::Static)
		//					{
		//						if (IS_SET(InGui::DropdownItem("Static"), InGuiReturnType::Clicked))
		//						{
		//							rigidBody.Type = RigidBody2DComponent::BodyType::Static;
		//							m_RigidBodyTypeOpen = false;
		//						}
		//					}
		//					if (rigidBody.Type != RigidBody2DComponent::BodyType::Dynamic)
		//					{
		//						if (IS_SET(InGui::DropdownItem("Dynamic"), InGuiReturnType::Clicked))
		//						{
		//							rigidBody.Type = RigidBody2DComponent::BodyType::Dynamic;
		//							m_RigidBodyTypeOpen = false;
		//						}
		//					}
		//					if (rigidBody.Type != RigidBody2DComponent::BodyType::Kinematic)
		//					{
		//						if (IS_SET(InGui::DropdownItem("Kinematic"), InGuiReturnType::Clicked))
		//						{
		//							rigidBody.Type = RigidBody2DComponent::BodyType::Kinematic;
		//							m_RigidBodyTypeOpen = false;
		//						}
		//					}	
		//				}
		//				InGui::EndDropdown();
		//			}
		//			InGui::Separator();
		//		}
		//		if (m_Context.HasComponent<BoxCollider2DComponent>())
		//		{
		//			BoxCollider2DComponent& boxCollider = m_Context.GetComponent<BoxCollider2DComponent>();
		//			if (InGui::BeginGroup("BoxCollider2D", size, m_ComponentGroups[index++]))
		//			{
		//				InGui::Text("Offset");
		//				InGui::Separator();
		//				InGui::Float("X", glm::vec2(50.0f, 30.0f), boxCollider.Offset.x);
		//				InGui::Float("Y", glm::vec2(50.0f, 30.0f), boxCollider.Offset.y);
		//				InGui::Separator();
		//
		//				InGui::Text("Size");
		//				InGui::Separator();
		//				InGui::Float("X", glm::vec2(50.0f, 30.0f), boxCollider.Size.x);
		//				InGui::Float("Y", glm::vec2(50.0f, 30.0f), boxCollider.Size.y);
		//				InGui::Separator();
		//
		//				InGui::Text("Density");
		//				InGui::Separator();
		//				InGui::Float("", glm::vec2(50.0f, 30.0f), boxCollider.Density);
		//				InGui::Separator();
		//			}
		//			InGui::Separator();
		//		}
		//		
		//		if (m_Context.HasComponent<ScriptComponent>())
		//		{
		//			ScriptComponent& script = m_Context.GetComponent<ScriptComponent>();
		//			if (InGui::BeginGroup("Script Component", size, m_ComponentGroups[index++]))
		//			{
		//				InGui::String("Module Name", glm::vec2(120.0f, 25.0f), script.ModuleName);
		//				InGui::Separator();
		//				InGui::Text("Public Fields:");
		//				InGui::Separator();
		//				for (auto& field : script.Fields)
		//				{			
		//					if (field.GetType() == PublicFieldType::Float)
		//					{
		//						float val = field.GetStoredValue<float>();
		//						if (InGui::Float(field.GetName().c_str(), glm::vec2(50.0f, 30.0f), val) == InGuiReturnType::Modified)
		//						{
		//							field.SetStoredValue<float>(val);
		//							field.SetRuntimeValue<float>(val);
		//						}
		//					}
		//					else if (field.GetType() == PublicFieldType::Vec2)
		//					{
		//						glm::vec2 val = field.GetStoredValue<glm::vec2>();
		//						if (InGui::Float("", glm::vec2(50.0f, 30.0f), val.x) == InGuiReturnType::Modified)
		//						{
		//							field.SetStoredValue<glm::vec2>(val);
		//							field.SetRuntimeValue<glm::vec2>(val);
		//						}
		//						if (InGui::Float(field.GetName().c_str(), glm::vec2(50.0f, 30.0f), val.y) == InGuiReturnType::Modified)
		//						{
		//							field.SetStoredValue<glm::vec2>(val);
		//							field.SetRuntimeValue<glm::vec2>(val);
		//						}
		//					}
		//					else if (field.GetType() == PublicFieldType::Vec3)
		//					{
		//						glm::vec3 val = field.GetStoredValue<glm::vec3>();								
		//						if (InGui::Float("", glm::vec2(50.0f, 30.0f), val.x) == InGuiReturnType::Modified)
		//						{
		//							field.SetStoredValue<glm::vec3>(val);
		//							field.SetRuntimeValue<glm::vec3>(val);
		//						}
		//						if (InGui::Float("", glm::vec2(50.0f, 30.0f), val.y) == InGuiReturnType::Modified)
		//						{
		//							field.SetStoredValue<glm::vec3>(val);
		//							field.SetRuntimeValue<glm::vec3>(val);
		//						}
		//						if (InGui::Float(field.GetName().c_str(), glm::vec2(50.0f, 30.0f), val.z) == InGuiReturnType::Modified)
		//						{
		//							field.SetStoredValue<glm::vec3>(val);
		//							field.SetRuntimeValue<glm::vec3>(val);
		//						}
		//					}
		//					else if (field.GetType() == PublicFieldType::Vec4)
		//					{
		//						glm::vec4 val = field.GetStoredValue<glm::vec4>();
		//						if (InGui::Float("", glm::vec2(50.0f, 30.0f), val.x) == InGuiReturnType::Modified)
		//						{
		//							field.SetStoredValue<glm::vec4>(val);
		//							field.SetRuntimeValue<glm::vec4>(val);
		//						}
		//						if (InGui::Float("", glm::vec2(50.0f, 30.0f), val.y) == InGuiReturnType::Modified)
		//						{
		//							field.SetStoredValue<glm::vec4>(val);
		//							field.SetRuntimeValue<glm::vec4>(val);
		//						}
		//						if (InGui::Float("", glm::vec2(50.0f, 30.0f), val.z) == InGuiReturnType::Modified)
		//						{
		//							field.SetStoredValue<glm::vec4>(val);
		//							field.SetRuntimeValue<glm::vec4>(val);
		//						}
		//						if (InGui::Float(field.GetName().c_str(), glm::vec2(50.0f, 30.0f), val.w) == InGuiReturnType::Modified)
		//						{
		//							field.SetStoredValue<glm::vec4>(val);
		//							field.SetRuntimeValue<glm::vec4>(val);
		//						}
		//					}
		//					else if (field.GetType() == PublicFieldType::String)
		//					{
		//						std::string val = field.GetStoredValue<std::string>();
		//						if (InGui::String(field.GetName().c_str(), glm::vec2(50.0f, 30.0f), val) == InGuiReturnType::Modified)
		//						{
		//							field.SetStoredValue<std::string>(val);
		//							field.SetRuntimeValue<std::string>(val);			
		//						}
		//					}
		//					InGui::Separator();
		//				}
		//			}
		//			InGui::Separator();
		//		}
		//		m_ScrollScale = InGui::GetPositionOfNext().y - InGui::GetWindow(m_PanelID).Position.y + m_ScrollOffset;
		//		InGui::Separator();
		//		InGui::EndScrollableArea();
		//
		//		layout.LeftPadding = 60.0f;
		//		layout.RightPadding = 60.0f;
		//		InGui::SetLayout(m_PanelID, layout);
		//		InGui::Separator();
		//		InGui::SetTextCenter(InGuiTextCenter::Middle);
		//		InGui::Dropdown("Add Component", glm::vec2(InGui::GetWindow(m_PanelID).Size.x - 120.0f, 25.0f), m_AddComponentOpen);
		//		if (m_AddComponentOpen)
		//		{
		//			if (!m_Context.HasComponent<ScriptComponent>())
		//			{
		//				if (IS_SET(InGui::DropdownItem("Add Script Component"), InGuiReturnType::Clicked))
		//					m_AddComponentOpen = false;
		//			}
		//			if (!m_Context.HasComponent<RigidBody2DComponent>())
		//			{
		//				if (IS_SET(InGui::DropdownItem("Add RigidBody2D"), InGuiReturnType::Clicked))
		//					m_AddComponentOpen = false;
		//			}
		//			if (!m_Context.HasComponent<BoxCollider2DComponent>())
		//			{
		//				if (IS_SET(InGui::DropdownItem("Add BoxCollider2D"), InGuiReturnType::Clicked))
		//					m_AddComponentOpen = false;
		//			}
		//		}
		//		InGui::EndDropdown();
		//		
		//		layout.LeftPadding = 10.0f;
		//		layout.RightPadding = 10.0f;
		//		InGui::SetLayout(m_PanelID, layout);
		//		InGui::SetTextCenter(InGuiTextCenter::Left);
		//	}
		//}
		//InGui::End();
	//}
	size_t InspectorPanel::transformComponentUI(IGHierarchyElement& parent)
	{
		IGHierarchyElement element;
		element.Element = IGElementType::Group;		
		element.Children.push_back({ IGElementType::Float, {} });
		element.Children.push_back({ IGElementType::Float, {} });
		element.Children.push_back({ IGElementType::Float, {} });
		element.Children.push_back({ IGElementType::Text, {} }); // Translation
		element.Children.push_back({ IGElementType::Separator, {} });

		element.Children.push_back({ IGElementType::Float, {} });
		element.Children.push_back({ IGElementType::Float, {} });
		element.Children.push_back({ IGElementType::Float, {} });
		element.Children.push_back({ IGElementType::Text, {} }); // Rotation
		element.Children.push_back({ IGElementType::Separator, {} });

		element.Children.push_back({ IGElementType::Float, {} });
		element.Children.push_back({ IGElementType::Float, {} });
		element.Children.push_back({ IGElementType::Float, {} });
		element.Children.push_back({ IGElementType::Text, {} }); // Scale
		element.Children.push_back({ IGElementType::Separator, {} });
		parent.Children.push_back(element);
		return element.Children.size() + 1;
	}
	size_t InspectorPanel::spriteRendererUI(IGHierarchyElement& parent)
	{
		IGHierarchyElement element;
		element.Element = IGElementType::Group;	
		element.Children.push_back({ IGElementType::Float, {} });
		element.Children.push_back({ IGElementType::Float, {} });
		element.Children.push_back({ IGElementType::Float, {} });
		element.Children.push_back({ IGElementType::Float, {} });
		element.Children.push_back({ IGElementType::Separator, {} });
		element.Children.push_back({ IGElementType::Int, {} });
		element.Children.push_back({ IGElementType::Separator, {} });
		element.Children.push_back({ IGElementType::Checkbox, {} });
		parent.Children.push_back(element);
		return element.Children.size() + 1;
	}
	size_t InspectorPanel::pointLight2DUI(IGHierarchyElement& parent)
	{
		IGHierarchyElement element;
		return element.Children.size() + 1;
	}
	size_t InspectorPanel::rigidBody2DComponentUI(IGHierarchyElement& parent)
	{
		IGHierarchyElement element;
		return element.Children.size() + 1;
	}
	size_t InspectorPanel::boxCollider2DComponentUI(IGHierarchyElement& parent)
	{
		IGHierarchyElement element;
		return element.Children.size() + 1;
	}
	size_t InspectorPanel::scriptComponentUI(IGHierarchyElement& parent)
	{
		IGHierarchyElement element;
		return element.Children.size() + 1;
	}
	void InspectorPanel::prepareTransformComponentUI()
	{
		uint32_t handle = m_HandleStart[TransformComponent];
		IGGroup& transformGroup = IG::GetUI<IGGroup>(m_PoolHandle, m_Handles[handle]);
		if (transformGroup.Active = m_Context.HasComponent<XYZ::TransformComponent>())
		{
			XYZ::TransformComponent& transform = m_Context.GetComponent<XYZ::TransformComponent>();
			IGFloat& translationX = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 1]);
			IGFloat& translationY = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 2]);
			IGFloat& translationZ = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 3]);

			translationX.SetValue(transform.Translation.x);
			translationY.SetValue(transform.Translation.y);
			translationZ.SetValue(transform.Translation.z);

			IGFloat& rotationX = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 6]);
			IGFloat& rotationY = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 7]);
			IGFloat& rotationZ = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 8]);

			rotationX.SetValue(transform.Rotation.x);
			rotationY.SetValue(transform.Rotation.y);
			rotationZ.SetValue(transform.Rotation.z);

			IGFloat& scaleX = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 11]);
			IGFloat& scaleY = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 12]);
			IGFloat& scaleZ = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 13]);

			scaleX.SetValue(transform.Scale.x);
			scaleY.SetValue(transform.Scale.y);
			scaleZ.SetValue(transform.Scale.z);
		}
	}
	void InspectorPanel::prepareSpriteRendererUI()
	{
		uint32_t handle = m_HandleStart[SpriteRenderer];
		IGGroup& spriteRendererGroup = IG::GetUI<IGGroup>(m_PoolHandle, m_Handles[handle]);
		if (spriteRendererGroup.Active = m_Context.HasComponent<XYZ::SpriteRenderer>())
		{
			XYZ::SpriteRenderer& spriteRenderer = m_Context.GetComponent<XYZ::SpriteRenderer>();
			IGFloat& colorR = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 1]);
			IGFloat& colorG = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 2]);
			IGFloat& colorB = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 3]);
			IGFloat& colorA = IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 4]);
			IGInt& sortLayer = IG::GetUI<IGInt>(m_PoolHandle, m_Handles[handle + 6]);
			IGCheckbox& visible = IG::GetUI<IGCheckbox>(m_PoolHandle, m_Handles[handle + 8]);

			colorR.SetValue(spriteRenderer.Color.r);
			colorG.SetValue(spriteRenderer.Color.g);
			colorB.SetValue(spriteRenderer.Color.b);
			colorA.SetValue(spriteRenderer.Color.a);
			sortLayer.SetValue(spriteRenderer.SortLayer);
			visible.Checked = spriteRenderer.IsVisible;
		}
	}
	void InspectorPanel::setupTransformComponentUI()
	{
		Ref<Font> font = IG::GetContext().RenderData.Font;
		uint32_t handle = m_HandleStart[TransformComponent];
		IG::GetUI<IGGroup>(m_PoolHandle, m_Handles[handle]).Label = "Transform Component";
		
		IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 1]).Label = "X";
		IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 2]).Label = "Y";
		IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 3]).Label = "Z";
		IG::GetUI<IGText>(m_PoolHandle, m_Handles[handle + 4]).Text = "Translation";
		IG::GetUI<IGText>(m_PoolHandle, m_Handles[handle + 4]).Size.x = Helper::FindTextLength("Translation", font) + 1.0f;

		IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 6]).Label = "X";
		IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 7]).Label = "Y";
		IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 8]).Label = "Z";
		IG::GetUI<IGText>(m_PoolHandle, m_Handles[handle + 9]).Text = "Rotation";
		IG::GetUI<IGText>(m_PoolHandle, m_Handles[handle + 9]).Size.x = Helper::FindTextLength("Rotation", font) + 1.0f;

		IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 11]).Label = "X";
		IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 12]).Label = "Y";
		IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 13]).Label = "Z";
		IG::GetUI<IGText>(m_PoolHandle, m_Handles[handle + 14]).Text = "Scale";
		IG::GetUI<IGText>(m_PoolHandle, m_Handles[handle + 14]).Size.x = Helper::FindTextLength("Scale", font) + 1.0f;
	}
	void InspectorPanel::setupSpriteRendererUI()
	{
		Ref<Font> font = IG::GetContext().RenderData.Font;
		uint32_t handle = m_HandleStart[SpriteRenderer];

		IG::GetUI<IGGroup>(m_PoolHandle, m_Handles[handle]).Label = "Sprite Renderer";
		IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 1]).Label = "R";
		IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 2]).Label = "G";
		IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 3]).Label = "B";
		IG::GetUI<IGFloat>(m_PoolHandle, m_Handles[handle + 4]).Label = "A";
		
		IG::GetUI<IGInt>(m_PoolHandle, m_Handles[handle + 6]).Label = "Sort Layer";
		IG::GetUI<IGCheckbox>(m_PoolHandle, m_Handles[handle + 8]).Label = "Visible";
		IG::GetUI<IGCheckbox>(m_PoolHandle, m_Handles[handle + 8]).Size = { 25.0f, 25.0f };
	}
	void InspectorPanel::setupPointLight2DUI()
	{
	}
	void InspectorPanel::setupRigidBody2DComponentUI()
	{
	}
	void InspectorPanel::setupBoxCollider2DComponentUI()
	{
	}
	void InspectorPanel::setupScriptComponentUI()
	{
	}
}