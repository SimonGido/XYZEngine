#include "stdafx.h"
#include "InspectorPanel.h"

#include "XYZ/Renderer/Font.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Utils/FileSystem.h"
#include "XYZ/Utils/StringUtils.h"
#include "XYZ/Script/ScriptEngine.h"

#include <imgui.h>
#include <imgui_internal.h>

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

		static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
		{
			ImGuiIO& io = ImGui::GetIO();
			auto boldFont = io.Fonts->Fonts[0];

			ImGui::PushID(label.c_str());

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, columnWidth);
			ImGui::Text(label.c_str());
			ImGui::NextColumn();

	
			ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f });
			ImGui::PushFont(boldFont);
			if (ImGui::Button("X", buttonSize))
				values.x = resetValue;
			ImGui::PopFont();
			

			ImGui::SameLine();
			ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::PushFont(boldFont);
			if (ImGui::Button("Y", buttonSize))
				values.y = resetValue;
			ImGui::PopFont();


			ImGui::SameLine();
			ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();


			ImGui::PushFont(boldFont);
			if (ImGui::Button("Z", buttonSize))
				values.z = resetValue;
			ImGui::PopFont();
			
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();

			ImGui::PopStyleVar();

			ImGui::Columns(1);

			ImGui::PopID();
		}

		template<typename T, typename UIFunction>
		static void DrawComponent(const std::string& name, SceneEntity entity, UIFunction uiFunction)
		{
			const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
			if (entity.HasComponent<T>())
			{
				auto& component = entity.GetComponent<T>();
				ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
				ImGui::Separator();
				bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
				ImGui::PopStyleVar(
				);
				ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
				if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
				{
					ImGui::OpenPopup("ComponentSettings");
				}

				bool removeComponent = false;
				if (ImGui::BeginPopup("ComponentSettings"))
				{
					if (ImGui::MenuItem("Remove component"))
						removeComponent = true;

					ImGui::EndPopup();
				}

				if (open)
				{
					uiFunction(component);
					ImGui::TreePop();
				}

				if (removeComponent)
					entity.RemoveComponent<T>();
			}
		}
	}
	namespace Editor {

		void InspectorPanel::OnImGuiRender()
		{
			if (ImGui::Begin("Inspector"))
			{
				if (m_Context && m_Context.IsValid())
				{
					Helper::DrawComponent<SceneTagComponent>("Scene Tag", m_Context, [&](auto& component) {

						std::string& tag = m_Context.GetComponent<SceneTagComponent>().Name;
						char buffer[256];
						memset(buffer, 0, sizeof(buffer));
						std::strncpy(buffer, tag.c_str(), sizeof(buffer));
						if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
						{
							tag = std::string(buffer);
						}
					});
					
					Helper::DrawComponent<TransformComponent>("Transform", m_Context, [&](auto& component) {
						
						Helper::DrawVec3Control("Translation", component.Translation);
						glm::vec3 rotation = glm::degrees(component.Rotation);
						Helper::DrawVec3Control("Rotation", rotation);
						component.Rotation = glm::radians(rotation);
						Helper::DrawVec3Control("Scale", component.Scale, 1.0f);
					});
					
					//if (m_Context.HasComponent<SpriteRenderer>())
					//{
					//	if (InGui::Begin("Sprite Renderer", flags))
					//	{
					//		SpriteRenderer& sprite = m_Context.GetComponent<SpriteRenderer>();
					//		InGui::Float4("R", "G", "B", "A Color", glm::vec2(40.0f, 25.0f), glm::value_ptr(sprite.Color), 2);
					//		InGui::String("Material", glm::vec2(150.0f, 25.0f), sprite.Material->FileName);
					//		InGui::UInt("Sort Layer", glm::vec2(40.0f, 25.0f), sprite.SortLayer);
					//		InGui::Checkbox("Visible", glm::vec2(25.0f, 25.0f), sprite.IsVisible);
					//	}
					//	InGui::End();
					//	InGuiWindow* componentWindow = InGui::GetContext().GetInGuiWindow("Sprite Renderer");
					//	componentWindow->Size.x = width;
					//	componentWindow->Size.y = componentWindow->FrameData.CursorPos.y - componentWindow->Position.y;
					//}
					//if (m_Context.HasComponent<ScriptComponent>())
					//{
					//
					//}
					//if (m_Context.HasComponent<CameraComponent>())
					//{
					//
					//}
				}
			}
			ImGui::End();		
		}
	

		InspectorPanel::InspectorPanel()
		{
		}

		void InspectorPanel::SetContext(SceneEntity context)
		{
			m_Context = context;
		}

		void InspectorPanel::drawComponents(SceneEntity entity)
		{
		}

	}
}