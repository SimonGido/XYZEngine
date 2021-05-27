#include "stdafx.h"
#include "InspectorPanel.h"

#include "XYZ/Core/Application.h"
#include "XYZ/Utils/FileSystem.h"
#include "XYZ/Utils/StringUtils.h"

#include "XYZ/Script/ScriptEngine.h"

#include "XYZ/InGui/InGui.h"

#include <glm/gtc/type_ptr.hpp>

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
	namespace Editor {

		void InspectorPanel::OnUpdate()
		{
			if (InGui::Begin("Inspector",			
				 InGuiWindowStyleFlags::PanelEnabled
				| InGuiWindowStyleFlags::ScrollEnabled
				| InGuiWindowStyleFlags::LabelEnabled
				| InGuiWindowStyleFlags::DockingEnabled
			))
			{
				InGuiWindow* inspector = InGui::GetContext().GetInGuiWindow("Inspector");
				float width =
					inspector->Size.x
					- 2.0f * InGui::GetContext().m_Config.WindowPadding.x
					- 2.0f * inspector->ScrollBarSize.y;

				if (m_Context && m_Context.IsValid())
				{
					InGuiWindowFlags flags = InGuiWindowStyleFlags::PanelEnabled
										   | InGuiWindowStyleFlags::LabelEnabled;
					if (m_Context.HasComponent<SceneTagComponent>())
					{		
						if (InGui::Begin("Scene Tag Component", flags))
						{
							InGui::String("Name", glm::vec2(150.0f, 25.0f), m_Context.GetComponent<SceneTagComponent>().Name);
						}
						InGui::End();
						InGuiWindow* componentWindow = InGui::GetContext().GetInGuiWindow("Scene Tag Component");
						componentWindow->Size.x = width;
						componentWindow->Size.y = componentWindow->FrameData.CursorPos.y - componentWindow->Position.y;
					}
					if (m_Context.HasComponent<TransformComponent>())
					{
						if (InGui::Begin("Transform Component", flags))
						{
							TransformComponent& transform = m_Context.GetComponent<TransformComponent>();
							InGui::Float3("X", "Y", "Z Translation", glm::vec2(40.0f, 25.0f), glm::value_ptr(transform.Translation), 2);
							InGui::Float3("#X", "#Y", "#Z Rotation", glm::vec2(40.0f, 25.0f), glm::value_ptr(transform.Rotation), 2);
							InGui::Float3("##X", "##Y", "##Z Scale", glm::vec2(40.0f, 25.0f), glm::value_ptr(transform.Scale), 2);
						}
						InGui::End();
						InGuiWindow* componentWindow = InGui::GetContext().GetInGuiWindow("Transform Component");
						componentWindow->Size.x = width;
						componentWindow->Size.y = componentWindow->FrameData.CursorPos.y - componentWindow->Position.y;
					}
					if (m_Context.HasComponent<SpriteRenderer>())
					{
						if (InGui::Begin("Sprite Renderer", flags))
						{
							SpriteRenderer& sprite = m_Context.GetComponent<SpriteRenderer>();
							InGui::Float4("R", "G", "B", "A Color", glm::vec2(40.0f, 25.0f), glm::value_ptr(sprite.Color), 2);
							InGui::String("Material", glm::vec2(150.0f, 25.0f), sprite.Material->FileName);
							InGui::UInt("Sort Layer", glm::vec2(40.0f, 25.0f), sprite.SortLayer);
							InGui::Checkbox("Visible", glm::vec2(25.0f, 25.0f), sprite.IsVisible);
						}
						InGui::End();
						InGuiWindow* componentWindow = InGui::GetContext().GetInGuiWindow("Sprite Renderer");
						componentWindow->Size.x = width;
						componentWindow->Size.y = componentWindow->FrameData.CursorPos.y - componentWindow->Position.y;
					}
					if (m_Context.HasComponent<ScriptComponent>())
					{
	
					}
					if (m_Context.HasComponent<CameraComponent>())
					{

					}
				}
			}
			InGui::End();		
		}
	

		InspectorPanel::InspectorPanel()
		{
		}

		void InspectorPanel::SetContext(SceneEntity context)
		{
			m_Context = context;
		}

	}
}