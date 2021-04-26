#include "stdafx.h"
#include "InspectorPanel.h"

#include "XYZ/BasicUI/BasicUILoader.h"


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
		m_TransformLayout = { 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, {3, 3, 3}, true };
		m_SpriteRendererLayout = { 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, {4, 1, 1}, true };
		m_ScriptLayout = { 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, {1}, true };
		m_SceneTagLayout =  { 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, {1}, true };

		bUILoader::Load("Layouts/Inspector.bui");

		bUIAllocator& allocator = bUI::GetAllocator("Inspector");
		bUIScrollbox* scrollbox = allocator.GetElement<bUIScrollbox>("Scrollbox");
		bUI::ForEach<bUIWindow>(allocator, scrollbox, [&](bUIWindow& win) {
			win.Visible = false;
		});

		bUI::SetOnReloadCallback("Inspector", [&](bUIAllocator& allocator) {
			setContextUI();
		});
	}
	InspectorPanel::~InspectorPanel()
	{
		/* Before destroying Inspector just reload 
		it to destroy runtime created script component UI */
		bUILoader::Load("Layouts/Inspector.bui", false);
		bUILoader::Save("Inspector", "Layouts/Inspector.bui");
	}
	void InspectorPanel::SetContext(SceneEntity context)
	{
		if (!context)
		{
			bUIAllocator& allocator = bUI::GetAllocator("Inspector");
			bUIScrollbox* scrollbox = allocator.GetElement<bUIScrollbox>("Scrollbox");
			bUI::ForEach<bUIWindow>(allocator, scrollbox, [&](bUIWindow& win) {
				win.Visible = false;
			});
			m_Context = context;
			return;
		}
		else if (m_Context == context)
		{
			return;
		}
		m_Context = context;
		bUILoader::Load("Layouts/Inspector.bui");
		setContextUI();
	}
	void InspectorPanel::OnUpdate()
	{
		bUIAllocator& allocator = bUI::GetAllocator("Inspector");
		bUIScrollbox* scrollbox = allocator.GetElement<bUIScrollbox>("Scrollbox");
		bUIWindow* last = nullptr;
		bUI::ForEach<bUIWindow>(allocator, scrollbox, [&](bUIWindow& win) {
			win.Size.x = scrollbox->Size.x - 20.0f;
			if (last)
				win.Coords.y = last->Coords.y + last->GetSize().y + 10.0f;
			else
				win.Coords.y = 35.0f;
			last = &win;
		});
		
		if (m_Context && m_Context.IsValid())
		{
			if (m_Context.HasComponent<SceneTagComponent>())
			{
				bUIWindow* sceneTagWindow = allocator.GetElement<bUIWindow>("Scene Tag Component");
				bUI::SetupLayout(allocator, *sceneTagWindow, m_SceneTagLayout);
			}
			if (m_Context.HasComponent<TransformComponent>())
			{
				bUIWindow* transformWindow = allocator.GetElement<bUIWindow>("Transform Component");
				bUI::SetupLayout(allocator, *transformWindow, m_TransformLayout);
			}
			if (m_Context.HasComponent<SpriteRenderer>())
			{
				bUIWindow* spriteWindow = allocator.GetElement<bUIWindow>("Sprite Renderer");
				bUI::SetupLayout(allocator, *spriteWindow, m_SpriteRendererLayout);
			}
			if (m_Context.HasComponent<ScriptComponent>())
			{
				bUIWindow* scriptWindow = allocator.GetElement<bUIWindow>("Script Component");
				bUI::SetupLayout(allocator, *scriptWindow, m_ScriptLayout);
			}
		}	
	}

	void InspectorPanel::setContextUI()
	{
		if (m_Context && m_Context.IsValid())
		{
			setSceneTagComponent();
			setTransformComponent();
			setSpriteRenderer();

			buildScriptComponent();
			setScriptComponent();

			bUIAllocator& allocator = bUI::GetAllocator("Inspector");
			bUIScrollbox* scrollbox = allocator.GetElement<bUIScrollbox>("Scrollbox");
			bUI::ForEach<bUIWindow>(allocator, scrollbox, [&](bUIWindow& win) {
				win.ChildrenVisible = false;
				});
		}
	}

	void InspectorPanel::setSceneTagComponent()
	{
		bUIAllocator& allocator = bUI::GetAllocator("Inspector");
		if (m_Context.HasComponent<SceneTagComponent>())
		{
			auto& component = m_Context.GetComponent<SceneTagComponent>();
			bUIString* sceneTag = allocator.GetElement<bUIString>("Scene Tag");
			sceneTag->SetValue(component.Name);
			sceneTag->Callbacks.push_back(
				[&](bUICallbackType type, bUIElement& element) {
					if (type == bUICallbackType::Active && m_Context)
					{
						bUIString& casted = static_cast<bUIString&>(element);
						m_Context.GetComponent<SceneTagComponent>().Name = casted.GetValue();
					}
				}
			);
		}
	}

	void InspectorPanel::setTransformComponent()
	{
		bUIAllocator& allocator = bUI::GetAllocator("Inspector");
		if (m_Context.HasComponent<TransformComponent>())
		{
			auto& component = m_Context.GetComponent<TransformComponent>();
			
			// Translation
			bUIFloat* translationX = allocator.GetElement<bUIFloat>("Translation X");
			translationX->SetValue(component.Translation.x);
			translationX->Callbacks.push_back(
				[&](bUICallbackType type, bUIElement& element) {
					if (type == bUICallbackType::Active && m_Context)
					{
						bUIFloat& casted = static_cast<bUIFloat&>(element);
						m_Context.GetComponent<TransformComponent>().Translation.x = casted.GetValue();
					}
				}
			);
			bUIFloat* translationY = allocator.GetElement<bUIFloat>("Translation Y");
			translationY->SetValue(component.Translation.y);
			translationY->Callbacks.push_back(
				[&](bUICallbackType type, bUIElement& element) {
					if (type == bUICallbackType::Active && m_Context)
					{
						bUIFloat& casted = static_cast<bUIFloat&>(element);
						m_Context.GetComponent<TransformComponent>().Translation.y = casted.GetValue();
					}
				}
			);

			bUIFloat* translationZ = allocator.GetElement<bUIFloat>("Translation Z");
			translationZ->SetValue(component.Translation.z);
			translationZ->Callbacks.push_back(
				[&](bUICallbackType type, bUIElement& element) {
					if (type == bUICallbackType::Active && m_Context)
					{
						bUIFloat& casted = static_cast<bUIFloat&>(element);
						m_Context.GetComponent<TransformComponent>().Translation.z = casted.GetValue();
					}
				}
			);
			// Translation

			// Rotation
			bUIFloat* rotationX = allocator.GetElement<bUIFloat>("Rotation X");
			rotationX->SetValue(component.Rotation.x);
			rotationX->Callbacks.push_back(
				[&](bUICallbackType type, bUIElement& element) {
					if (type == bUICallbackType::Active && m_Context)
					{
						bUIFloat& casted = static_cast<bUIFloat&>(element);
						m_Context.GetComponent<TransformComponent>().Rotation.x = casted.GetValue();
					}
				}
			);
			bUIFloat* rotationY = allocator.GetElement<bUIFloat>("Rotation Y");
			rotationY->SetValue(component.Rotation.y);
			rotationY->Callbacks.push_back(
				[&](bUICallbackType type, bUIElement& element) {
					if (type == bUICallbackType::Active && m_Context)
					{
						bUIFloat& casted = static_cast<bUIFloat&>(element);
						m_Context.GetComponent<TransformComponent>().Rotation.y = casted.GetValue();
					}
				}
			);

			bUIFloat* rotationZ = allocator.GetElement<bUIFloat>("Rotation Z");
			rotationZ->SetValue(component.Rotation.z);
			rotationZ->Callbacks.push_back(
				[&](bUICallbackType type, bUIElement& element) {
					if (type == bUICallbackType::Active && m_Context)
					{
						bUIFloat& casted = static_cast<bUIFloat&>(element);
						m_Context.GetComponent<TransformComponent>().Rotation.z = casted.GetValue();
					}
				}
			);
			// Rotation

			// Scale
			bUIFloat* scaleX = allocator.GetElement<bUIFloat>("Scale X");
			scaleX->SetValue(component.Scale.x);
			scaleX->Callbacks.push_back(
				[&](bUICallbackType type, bUIElement& element) {
					if (type == bUICallbackType::Active && m_Context)
					{
						bUIFloat& casted = static_cast<bUIFloat&>(element);
						m_Context.GetComponent<TransformComponent>().Scale.x = casted.GetValue();
					}
				}
			);
			bUIFloat* scaleY = allocator.GetElement<bUIFloat>("scale Y");
			scaleY->SetValue(component.Scale.y);
			scaleY->Callbacks.push_back(
				[&](bUICallbackType type, bUIElement& element) {
					if (type == bUICallbackType::Active && m_Context)
					{
						bUIFloat& casted = static_cast<bUIFloat&>(element);
						m_Context.GetComponent<TransformComponent>().Scale.y = casted.GetValue();
					}
				}
			);

			bUIFloat* scaleZ = allocator.GetElement<bUIFloat>("scale Z");
			scaleZ->SetValue(component.Scale.z);
			scaleZ->Callbacks.push_back(
				[&](bUICallbackType type, bUIElement& element) {
					if (type == bUICallbackType::Active && m_Context)
					{
						bUIFloat& casted = static_cast<bUIFloat&>(element);
						m_Context.GetComponent<TransformComponent>().Scale.z = casted.GetValue();
					}
				}
			);
			// Scale
		}
		else
		{
			bUIWindow* window = allocator.GetElement<bUIWindow>("Transform Component");
			window->Visible = false;
		}
	}

	void InspectorPanel::setSpriteRenderer()
	{
		bUIAllocator& allocator = bUI::GetAllocator("Inspector");
		if (m_Context.HasComponent<SpriteRenderer>())
		{
			auto& component = m_Context.GetComponent<SpriteRenderer>();

			// Color
			bUIFloat* colorR = allocator.GetElement<bUIFloat>("Color R");
			colorR->SetValue(component.Color.r);
			colorR->Callbacks.push_back(
				[&](bUICallbackType type, bUIElement& element) {
					if (type == bUICallbackType::Active && m_Context)
					{
						bUIFloat& casted = static_cast<bUIFloat&>(element);
						m_Context.GetComponent<SpriteRenderer>().Color.r = casted.GetValue();
					}
				}
			);

			bUIFloat* colorG = allocator.GetElement<bUIFloat>("Color G");
			colorG->SetValue(component.Color.g);
			colorG->Callbacks.push_back(
				[&](bUICallbackType type, bUIElement& element) {
					if (type == bUICallbackType::Active && m_Context)
					{
						bUIFloat& casted = static_cast<bUIFloat&>(element);
						m_Context.GetComponent<SpriteRenderer>().Color.g = casted.GetValue();
					}
				}
			);

			bUIFloat* colorB = allocator.GetElement<bUIFloat>("Color B");
			colorB->SetValue(component.Color.b);
			colorB->Callbacks.push_back(
				[&](bUICallbackType type, bUIElement& element) {
					if (type == bUICallbackType::Active && m_Context)
					{
						bUIFloat& casted = static_cast<bUIFloat&>(element);
						m_Context.GetComponent<SpriteRenderer>().Color.b = casted.GetValue();
					}
				}
			);

			bUIFloat* colorA = allocator.GetElement<bUIFloat>("Color A");
			colorA->SetValue(component.Color.a);
			colorA->Callbacks.push_back(
				[&](bUICallbackType type, bUIElement& element) {
					if (type == bUICallbackType::Active && m_Context)
					{
						bUIFloat& casted = static_cast<bUIFloat&>(element);
						m_Context.GetComponent<SpriteRenderer>().Color.a = casted.GetValue();
					}
				}
			);
			// Color

			// Sort Layer
			bUIInt* sortLayer = allocator.GetElement<bUIInt>("Sort Layer");
			sortLayer->SetValue(component.SortLayer);
			sortLayer->Callbacks.push_back(
				[&](bUICallbackType type, bUIElement& element) {
					if (type == bUICallbackType::Active && m_Context)
					{
						bUIInt& casted = static_cast<bUIInt&>(element);
						m_Context.GetComponent<SpriteRenderer>().SortLayer = casted.GetValue();
					}
				}
			);
			// Sort Layer

			// Visible
			bUICheckbox* checkbox = allocator.GetElement<bUICheckbox>("Visible");
			checkbox->Checked = component.IsVisible;
			checkbox->Callbacks.push_back(
				[&](bUICallbackType type, bUIElement& element) {
					if (type == bUICallbackType::StateChange && m_Context)
					{
						bUICheckbox& casted = static_cast<bUICheckbox&>(element);
						m_Context.GetComponent<SpriteRenderer>().IsVisible = casted.Checked;
					}
				}
			);
			// Visible
		}
		else
		{
			bUIWindow* window = allocator.GetElement<bUIWindow>("Sprite Renderer");
			window->Visible = false;
		}
	}

	void InspectorPanel::setScriptComponent()
	{
		if (m_Context.HasComponent<ScriptComponent>())
		{
			bUIAllocator& allocator = bUI::GetAllocator("Inspector");
			ScriptComponent& scriptComponent = m_Context.GetComponent<ScriptComponent>();
			for (auto& field : scriptComponent.Fields)
			{
				if (field.GetType() == PublicFieldType::Float)
				{
					bUIFloat* element = allocator.GetElement<bUIFloat>(field.GetName());
					element->SetValue(field.GetRuntimeValue<float>());
					element->Callbacks.push_back(
						[&](bUICallbackType type, bUIElement& elem) {
							if (type == bUICallbackType::Active && m_Context)
							{
								bUIFloat& casted = static_cast<bUIFloat&>(elem);
								field.SetStoredValue<float>(casted.GetValue());
							}
						}
					);
				}
				else if (field.GetType() == PublicFieldType::Int)
				{
					bUIInt* element = allocator.GetElement<bUIInt>(field.GetName());
					element->SetValue(field.GetRuntimeValue<int32_t>());
					element->Callbacks.push_back(
						[&](bUICallbackType type, bUIElement& elem) {
							if (type == bUICallbackType::Active && m_Context)
							{
								bUIInt& casted = static_cast<bUIInt&>(elem);
								field.SetStoredValue<int32_t>(casted.GetValue());
							}
						}
					);
				}
				else if (field.GetType() == PublicFieldType::UnsignedInt)
				{
					bUIInt* element = allocator.GetElement<bUIInt>(field.GetName());
					element->SetValue(field.GetRuntimeValue<uint32_t>());
					element->Callbacks.push_back(
						[&](bUICallbackType type, bUIElement& elem) {
							if (type == bUICallbackType::Active && m_Context)
							{
								bUIInt& casted = static_cast<bUIInt&>(elem);
								field.SetStoredValue<uint32_t>((uint32_t)casted.GetValue());
							}
						}
					);
				}
				else if (field.GetType() == PublicFieldType::Vec2)
				{
					glm::vec2 val = field.GetRuntimeValue<glm::vec2>();
					bUIFloat* elementX = allocator.GetElement<bUIFloat>(field.GetName() + "X");
					elementX->SetValue(val.x);
					elementX->Callbacks.push_back(
						[&](bUICallbackType type, bUIElement& elem) {
							if (type == bUICallbackType::Active && m_Context)
							{
								bUIFloat& casted = static_cast<bUIFloat&>(elem);
								glm::vec2 val = field.GetStoredValue<glm::vec2>();
								val.x = casted.GetValue();
								field.SetStoredValue<glm::vec2>(val);
							}
						}
					);

					bUIFloat* elementY = allocator.GetElement<bUIFloat>(field.GetName() + "Y");
					elementY->SetValue(val.y);
					elementY->Callbacks.push_back(
						[&](bUICallbackType type, bUIElement& elem) {
							if (type == bUICallbackType::Active && m_Context)
							{
								bUIFloat& casted = static_cast<bUIFloat&>(elem);
								glm::vec2 val = field.GetStoredValue<glm::vec2>();
								val.y = casted.GetValue();
								field.SetStoredValue<glm::vec2>(val);
							}
						}
					);
				}
				else if (field.GetType() == PublicFieldType::Vec3)
				{
					glm::vec3 val = field.GetRuntimeValue<glm::vec3>();
					bUIFloat* elementX = allocator.GetElement<bUIFloat>(field.GetName() + "X");
					elementX->SetValue(val.x);
					elementX->Callbacks.push_back(
						[&](bUICallbackType type, bUIElement& elem) {
							if (type == bUICallbackType::Active && m_Context)
							{
								bUIFloat& casted = static_cast<bUIFloat&>(elem);
								glm::vec3 val = field.GetStoredValue<glm::vec3>();
								val.x = casted.GetValue();
								field.SetStoredValue<glm::vec3>(val);
							}
						}
					);
					bUIFloat* elementY = allocator.GetElement<bUIFloat>(field.GetName() + "Y");
					elementY->SetValue(val.y);
					elementY->Callbacks.push_back(
						[&](bUICallbackType type, bUIElement& elem) {
							if (type == bUICallbackType::Active && m_Context)
							{
								bUIFloat& casted = static_cast<bUIFloat&>(elem);
								glm::vec3 val = field.GetStoredValue<glm::vec3>();
								val.y = casted.GetValue();
								field.SetStoredValue<glm::vec3>(val);
							}
						}
					);
					bUIFloat* elementZ = allocator.GetElement<bUIFloat>(field.GetName() + "Z");
					elementZ->SetValue(val.z);
					elementZ->Callbacks.push_back(
						[&](bUICallbackType type, bUIElement& elem) {
							if (type == bUICallbackType::Active && m_Context)
							{
								bUIFloat& casted = static_cast<bUIFloat&>(elem);
								glm::vec3 val = field.GetStoredValue<glm::vec3>();
								val.z = casted.GetValue();
								field.SetStoredValue<glm::vec3>(val);
							}
						}
					);
				}
				else if (field.GetType() == PublicFieldType::Vec4)
				{
					glm::vec4 val = field.GetRuntimeValue<glm::vec4>();
					bUIFloat* elementX = allocator.GetElement<bUIFloat>(field.GetName() + "X");
					elementX->SetValue(val.x);
					elementX->Callbacks.push_back(
						[&](bUICallbackType type, bUIElement& elem) {
							if (type == bUICallbackType::Active && m_Context)
							{
								bUIFloat& casted = static_cast<bUIFloat&>(elem);
								glm::vec4 val = field.GetStoredValue<glm::vec4>();
								val.x = casted.GetValue();
								field.SetStoredValue<glm::vec4>(val);
							}
						}
					);

					bUIFloat* elementY = allocator.GetElement<bUIFloat>(field.GetName() + "Y");
					elementY->SetValue(val.y);
					elementY->Callbacks.push_back(
						[&](bUICallbackType type, bUIElement& elem) {
							if (type == bUICallbackType::Active && m_Context)
							{
								bUIFloat& casted = static_cast<bUIFloat&>(elem);
								glm::vec4 val = field.GetStoredValue<glm::vec4>();
								val.y = casted.GetValue();
								field.SetStoredValue<glm::vec4>(val);
							}
						}
					);

					bUIFloat* elementZ = allocator.GetElement<bUIFloat>(field.GetName() + "Z");
					elementZ->SetValue(val.z);
					elementZ->Callbacks.push_back(
						[&](bUICallbackType type, bUIElement& elem) {
							if (type == bUICallbackType::Active && m_Context)
							{
								bUIFloat& casted = static_cast<bUIFloat&>(elem);
								glm::vec4 val = field.GetStoredValue<glm::vec4>();
								val.z = casted.GetValue();
								field.SetStoredValue<glm::vec4>(val);
							}
						}
					);


					bUIFloat* elementW = allocator.GetElement<bUIFloat>(field.GetName() + "W");
					elementW->SetValue(val.w);
					elementW->Callbacks.push_back(
						[&](bUICallbackType type, bUIElement& elem) {
							if (type == bUICallbackType::Active && m_Context)
							{
								bUIFloat& casted = static_cast<bUIFloat&>(elem);
								glm::vec4 val = field.GetStoredValue<glm::vec4>();
								val.w = casted.GetValue();
								field.SetStoredValue<glm::vec4>(val);
							}
						}
					);

				}
				else if (field.GetType() == PublicFieldType::String)
				{
					bUIString* element = allocator.GetElement<bUIString>(field.GetName());
					element->SetValue(field.GetStoredValue<char*>());
					element->Callbacks.push_back(
						[&](bUICallbackType type, bUIElement& elem) {
							if (type == bUICallbackType::Active && m_Context)
							{
								bUIString& casted = static_cast<bUIString&>(elem);
								std::string val = casted.GetValue();
								field.SetStoredValue<const char*>(val.c_str());
							}
						}
					);
				}
			}
		}
	}

	void InspectorPanel::buildScriptComponent()
	{
		if (m_Context.HasComponent<ScriptComponent>())
		{
			bUIAllocator& allocator = bUI::GetAllocator("Inspector");
			ScriptComponent& scriptComponent = m_Context.GetComponent<ScriptComponent>();
			m_ScriptLayout.ItemsPerRow.clear();
			for (auto& field : scriptComponent.Fields)
			{
				switch (field.GetType())
				{
				case PublicFieldType::Float:
					allocator.CreateElement<bUIFloat>("Script Component", glm::vec2(0.0f), glm::vec2(50.0f), glm::vec4(1.0f), field.GetName(), field.GetName(), bUIElementType::Float);
					m_ScriptLayout.ItemsPerRow.push_back(1);
					break;
				case PublicFieldType::Int:
					allocator.CreateElement<bUIInt>("Script Component", glm::vec2(0.0f), glm::vec2(50.0f), glm::vec4(1.0f), field.GetName(), field.GetName(), bUIElementType::Int);
					m_ScriptLayout.ItemsPerRow.push_back(1);
					break;
				case PublicFieldType::UnsignedInt:
					allocator.CreateElement<bUIInt>("Script Component", glm::vec2(0.0f), glm::vec2(50.0f), glm::vec4(1.0f), field.GetName(), field.GetName(), bUIElementType::Int);
					m_ScriptLayout.ItemsPerRow.push_back(1);
					break;
				case PublicFieldType::Vec2:
					allocator.CreateElement<bUIFloat>("Script Component", glm::vec2(0.0f), glm::vec2(50.0f), glm::vec4(1.0f), "", field.GetName() + "X", bUIElementType::Float);
					allocator.CreateElement<bUIFloat>("Script Component", glm::vec2(0.0f), glm::vec2(50.0f), glm::vec4(1.0f), field.GetName(), field.GetName() + "Y", bUIElementType::Float);
					m_ScriptLayout.ItemsPerRow.push_back(2);
					break;
				case PublicFieldType::Vec3:
					allocator.CreateElement<bUIFloat>("Script Component", glm::vec2(0.0f), glm::vec2(50.0f), glm::vec4(1.0f), "", field.GetName() + "X", bUIElementType::Float);
					allocator.CreateElement<bUIFloat>("Script Component", glm::vec2(0.0f), glm::vec2(50.0f), glm::vec4(1.0f), "", field.GetName() + "Y", bUIElementType::Float);
					allocator.CreateElement<bUIFloat>("Script Component", glm::vec2(0.0f), glm::vec2(50.0f), glm::vec4(1.0f), field.GetName(), field.GetName() + "Z", bUIElementType::Float);
					m_ScriptLayout.ItemsPerRow.push_back(3);
					break;
				case PublicFieldType::Vec4:
					allocator.CreateElement<bUIFloat>("Script Component", glm::vec2(0.0f), glm::vec2(50.0f), glm::vec4(1.0f), "", field.GetName() + "X", bUIElementType::Float);
					allocator.CreateElement<bUIFloat>("Script Component", glm::vec2(0.0f), glm::vec2(50.0f), glm::vec4(1.0f), "", field.GetName() + "Y", bUIElementType::Float);
					allocator.CreateElement<bUIFloat>("Script Component", glm::vec2(0.0f), glm::vec2(50.0f), glm::vec4(1.0f), "", field.GetName() + "Z", bUIElementType::Float);
					allocator.CreateElement<bUIFloat>("Script Component", glm::vec2(0.0f), glm::vec2(50.0f), glm::vec4(1.0f), field.GetName(), field.GetName() + "W", bUIElementType::Float);
					m_ScriptLayout.ItemsPerRow.push_back(4);
					break;
				case PublicFieldType::String:
					allocator.CreateElement<bUIString>("Script Component", glm::vec2(0.0f), glm::vec2(50.0f), glm::vec4(1.0f), field.GetName(), field.GetName(), bUIElementType::String);
					m_ScriptLayout.ItemsPerRow.push_back(1);
					break;
				default:
					break;
				}
			}
		}
	}
	
	//void InspectorPanel::updateScriptComponentUI()
	//{
	//	if (m_Context.HasComponent<XYZ::ScriptComponent>())
	//	{
	//		XYZ::ScriptComponent& scriptComponent = m_Context.GetComponent<XYZ::ScriptComponent>();
	//		size_t counter = 0;
	//		size_t index = m_HandleStart[ScriptComponent];
	//		IGPack& pack = IG::GetUI<IGPack>(m_PoolHandle, index + 1);
	//		for (auto& field : scriptComponent.Fields)
	//		{
	//			if (field.GetType() == PublicFieldType::Float)
	//			{
	//				IGFloat& fieldFloat = *static_cast<IGFloat*>(&pack[counter++]);
	//				fieldFloat.SetValue(field.GetStoredValue<float>());
	//				float newValue = fieldFloat.GetValue();
	//				field.SetStoredValue<float>(newValue);
	//				field.SetRuntimeValue<float>(newValue);
	//			}
	//			else if (field.GetType() == PublicFieldType::Int)
	//			{
	//				IGInt& fieldInt = *static_cast<IGInt*>(&pack[counter++]);
	//				fieldInt.SetValue(field.GetStoredValue<int32_t>());
	//				int32_t newValue = fieldInt.GetValue();
	//				field.SetStoredValue<int32_t>(newValue);
	//				field.SetRuntimeValue<int32_t>(newValue);
	//			}
	//			else if (field.GetType() == PublicFieldType::UnsignedInt)
	//			{
	//				IGInt& fieldInt = *static_cast<IGInt*>(&pack[counter++]);
	//				fieldInt.SetValue(field.GetStoredValue<int32_t>());
	//				int32_t newValue = fieldInt.GetValue();
	//				field.SetStoredValue<int32_t>(newValue);
	//				field.SetRuntimeValue<int32_t>(newValue);
	//			}
	//			else if (field.GetType() == PublicFieldType::Vec2)
	//			{
	//				IGFloat& firstFloat =  *static_cast<IGFloat*>(&pack[counter++]);
	//				IGFloat& secondFloat = *static_cast<IGFloat*>(&pack[counter++]);
	//
	//				glm::vec2 value = field.GetStoredValue<glm::vec2>();
	//
	//				firstFloat.SetValue(value.x);
	//				secondFloat.SetValue(value.y);
	//	
	//				field.SetStoredValue<glm::vec2>({ firstFloat.GetValue(), secondFloat.GetValue() });
	//				field.SetRuntimeValue<glm::vec2>({ firstFloat.GetValue(), secondFloat.GetValue() });
	//			}
	//			else if (field.GetType() == PublicFieldType::Vec3)
	//			{
	//				IGFloat& firstFloat  = *static_cast<IGFloat*>(&pack[counter++]);
	//				IGFloat& secondFloat = *static_cast<IGFloat*>(&pack[counter++]);
	//				IGFloat& thirdFloat  = *static_cast<IGFloat*>(&pack[counter++]);
	//				glm::vec3 value = field.GetStoredValue<glm::vec3>();
	//
	//				firstFloat.SetValue(value.x);
	//				secondFloat.SetValue(value.y);
	//				thirdFloat.SetValue(value.z);
	//
	//				field.SetStoredValue<glm::vec3>({ firstFloat.GetValue(), secondFloat.GetValue(), thirdFloat.GetValue() });
	//				field.SetRuntimeValue<glm::vec3>({ firstFloat.GetValue(), secondFloat.GetValue(), thirdFloat.GetValue() });
	//			}
	//			else if (field.GetType() == PublicFieldType::Vec4)
	//			{
	//				IGFloat& firstFloat  = *static_cast<IGFloat*>(&pack[counter++]);
	//				IGFloat& secondFloat = *static_cast<IGFloat*>(&pack[counter++]);
	//				IGFloat& thirdFloat  = *static_cast<IGFloat*>(&pack[counter++]);
	//				IGFloat& fourthFloat = *static_cast<IGFloat*>(&pack[counter++]);
	//				glm::vec4 value = field.GetStoredValue<glm::vec4>();
	//
	//				firstFloat.SetValue(value.x);
	//				secondFloat.SetValue(value.y);
	//				thirdFloat.SetValue(value.z);
	//				fourthFloat.SetValue(value.w);
	//
	//				field.SetStoredValue<glm::vec4>({ firstFloat.GetValue(), secondFloat.GetValue(), thirdFloat.GetValue(), fourthFloat.GetValue() });
	//				field.SetRuntimeValue<glm::vec4>({ firstFloat.GetValue(), secondFloat.GetValue(), thirdFloat.GetValue(), fourthFloat.GetValue() });
	//			}
	//			else if (field.GetType() == PublicFieldType::String)
	//			{
	//				IGString& str = *static_cast<IGString*>(&pack[counter++]);
	//				
	//				str.SetValue(field.GetStoredValue<char*>());
	//				std::string val = str.GetValue();
	//				field.SetStoredValue<const char*>(val.c_str());
	//				field.SetRuntimeValue<const char*>(val.c_str());
	//			}
	//		}
	//	}
	//}
}