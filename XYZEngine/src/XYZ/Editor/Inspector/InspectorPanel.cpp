#include "stdafx.h"
#include "InspectorPanel.h"

#include "XYZ/Renderer/Font.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Utils/FileSystem.h"
#include "XYZ/Utils/StringUtils.h"
#include "XYZ/Script/ScriptEngine.h"
#include "XYZ/Asset/AssetManager.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Scene/Components.h"

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
		static float FindTextLength(const char* source, Ref<Font> font)
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

		static void BeginColumns(const char* label, int count = 2, float width = 100.0f)
		{
			ImGui::Columns(count);
			ImGui::PushID(label);
			ImGui::SetColumnWidth(0, width);
			ImGui::Text(label);
			ImGui::NextColumn();
			
		}
		static void EndColumns()
		{
			ImGui::PopID();
			ImGui::Columns(1);
		}

		static void DrawFloatControl(const char* label, const char* dragLabel, float& value, float resetValue = 0.0f, float columnWidth = 100.0f)
		{
			ImGuiIO& io = ImGui::GetIO();
			auto boldFont = io.Fonts->Fonts[0];
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

			ImGui::PushFont(boldFont);
			if (ImGui::Button(label, buttonSize))
				value = resetValue;
			ImGui::PopFont();


			ImGui::SameLine();
			ImGui::DragFloat(dragLabel, &value, 0.05f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
		}

		static void DrawVec2Control(const std::string& label, glm::vec2& values, float resetValue = 0.0f, float columnWidth = 100.0f)
		{
			ImGuiIO& io = ImGui::GetIO();
			auto boldFont = io.Fonts->Fonts[0];
			BeginColumns(label.c_str());

			ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 5.0f });

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f });

			DrawFloatControl("X", "##X", values.x, resetValue, columnWidth);
			ImGui::SameLine();
			DrawFloatControl("Y", "##Y", values.y, resetValue, columnWidth);
			ImGui::PopStyleColor(3);

			ImGui::PopStyleVar();

			EndColumns();
		}

		static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
		{
			ImGuiIO& io = ImGui::GetIO();
			auto boldFont = io.Fonts->Fonts[0];
			BeginColumns(label.c_str());

			ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 5.0f });

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f });
			
			DrawFloatControl("X", "##X", values.x, resetValue, columnWidth);
			ImGui::SameLine();
			DrawFloatControl("Y", "##Y", values.y, resetValue, columnWidth);
			ImGui::SameLine();
			DrawFloatControl("Z", "##Z", values.z, resetValue, columnWidth);
			ImGui::PopStyleColor(3);

			ImGui::PopStyleVar();

			EndColumns();
		}

		static void DrawColorControl(const std::string& label, glm::vec4& values, float resetValue = 1.0f, float columnWidth = 100.0f)
		{
			ImGuiIO& io = ImGui::GetIO();
			auto boldFont = io.Fonts->Fonts[0];

			BeginColumns(label.c_str());

			ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 5.0f });

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f });

			DrawFloatControl("R", "##R", values.r, resetValue, columnWidth);
			ImGui::SameLine();
			DrawFloatControl("G", "##G", values.g, resetValue, columnWidth);
			ImGui::SameLine();
			DrawFloatControl("B", "##B", values.b, resetValue, columnWidth);
			ImGui::SameLine();
			DrawFloatControl("A", "##A", values.a, resetValue, columnWidth);


			ImGui::PopStyleColor(3);
			ImGui::PopStyleVar();
			EndColumns();
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
		InspectorPanel::InspectorPanel()
			:
			m_Context(nullptr)
		{
		}
		void InspectorPanel::OnImGuiRender()
		{
			if (ImGui::Begin("Inspector"))
			{
				if (m_Context)
					m_Context->OnImGuiRender();
			}
			ImGui::End();		
		}

		void InspectorPanel::SetContext(InspectorContext* context)
		{
			m_Context = context;
		}
	}
}