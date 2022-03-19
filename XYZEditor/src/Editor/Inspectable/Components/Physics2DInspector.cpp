#include "stdafx.h"
#include "Physics2DInspector.h"

#include "Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/Renderer/Renderer2D.h"

#include "XYZ/ImGui/ImGui.h"

#include "EditorLayer.h"

namespace XYZ {
	namespace Editor {
		RigidBody2DInspector::RigidBody2DInspector()
			:
			Inspectable("RigidBody2DInspector")
		{
		}

		bool RigidBody2DInspector::OnEditorRender()
		{
			return EditorHelper::DrawComponent<RigidBody2DComponent>("Rigid Body2D", m_Context, [&](auto& component) {

				if (ImGui::Button("Type"))
					ImGui::OpenPopup("Body Type");
				if (ImGui::BeginPopup("Body Type"))
				{
					if (ImGui::MenuItem("Static"))
					{
						component.Type = RigidBody2DComponent::BodyType::Static;
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::MenuItem("Dynamic"))
					{
						component.Type = RigidBody2DComponent::BodyType::Dynamic;
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::MenuItem("Kinematic"))
					{
						component.Type = RigidBody2DComponent::BodyType::Kinematic;
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				ImGui::SameLine();
				if (component.Type == RigidBody2DComponent::BodyType::Static)
					ImGui::Text("Static");
				else if (component.Type == RigidBody2DComponent::BodyType::Dynamic)
					ImGui::Text("Dynamic");
				else if (component.Type == RigidBody2DComponent::BodyType::Kinematic)
					ImGui::Text("Kinematic");
			});
		}

		void RigidBody2DInspector::SetSceneEntity(const SceneEntity& entity)
		{
			m_Context = entity;
		}

		BoxCollider2DInspector::BoxCollider2DInspector()
			:
			Inspectable("BoxCollider2DInspector")
		{
		}

		bool BoxCollider2DInspector::OnEditorRender()
		{
			return EditorHelper::DrawComponent<BoxCollider2DComponent>("Box Collider2D", m_Context, [&](auto& component) {

				if (ImGui::BeginTable("##BoxCollider2DTable", 2, ImGuiTableFlags_SizingStretchProp))
				{
					UI::ScopedStyleStack style(true, ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 5.0f });
					UI::ScopedColorStack color(true,
						ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f },
						ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f },
						ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f }
					);
					const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

					UI::TableRow("Size",
						[]() { ImGui::Text("Size"); },
						[&]() { UI::ScopedTableColumnAutoWidth scoped(2, lineHeight);
					UI::Vec2Control({ "X", "Y" }, component.Size); }
					);
					UI::TableRow("Offset",
						[]() { ImGui::Text("Offset"); },
						[&]() { UI::ScopedTableColumnAutoWidth scoped(2, lineHeight);
					UI::Vec2Control({ "X", "Y" }, component.Offset); }
					);
					UI::TableRow("Density",
						[]() { ImGui::Text("Density"); },
						[&]() { UI::ScopedTableColumnAutoWidth scoped(1, lineHeight);
					UI::FloatControl("##Density", "##DensityDrag", component.Density, 1.0f, 0.05f); }
					);
					UI::TableRow("Friction",
						[]() { ImGui::Text("Friction"); },
						[&]() { UI::ScopedTableColumnAutoWidth scoped(1, lineHeight);
					UI::FloatControl("##Friction", "##FrictionDrag", component.Friction, 1.0f, 0.05f); }
					);
					ImGui::EndTable();
				};
			});
		}

		void BoxCollider2DInspector::SetSceneEntity(const SceneEntity& entity)
		{
			m_Context = entity;
		}

		CircleCollider2DInspector::CircleCollider2DInspector()
			:
			Inspectable("CircleCollider2DInspector")
		{
		}

		bool CircleCollider2DInspector::OnEditorRender()
		{
			return EditorHelper::DrawComponent<CircleCollider2DComponent>("Circle Collider2D", m_Context, [&](auto& component) {

				if (ImGui::BeginTable("##CircleCollider2DTable", 2, ImGuiTableFlags_SizingStretchProp))
				{
					UI::ScopedStyleStack style(true, ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 5.0f });
					UI::ScopedColorStack color(true,
						ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f },
						ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f },
						ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f }
					);
					const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

					UI::TableRow("Offset",
						[]() { ImGui::Text("Offset"); },
						[&]() { UI::ScopedTableColumnAutoWidth scoped(2, lineHeight);
					UI::Vec2Control({ "X", "Y" }, component.Offset); }
					);

					UI::TableRow("Radius",
						[]() { ImGui::Text("Radius"); },
						[&]() { UI::ScopedTableColumnAutoWidth scoped(2, lineHeight);
					UI::FloatControl("##Radius", "##RadiusDrag", component.Radius, 1.0f, 0.05f); }
					);

					UI::TableRow("Density",
						[]() { ImGui::Text("Density"); },
						[&]() { UI::ScopedTableColumnAutoWidth scoped(1, lineHeight);
					UI::FloatControl("##Density", "##DensityDrag", component.Density, 1.0f, 0.05f); }
					);
					UI::TableRow("Friction",
						[]() { ImGui::Text("Friction"); },
						[&]() { UI::ScopedTableColumnAutoWidth scoped(1, lineHeight);
					UI::FloatControl("##Friction", "##FrictionDrag", component.Friction, 1.0f, 0.05f); }
					);
					ImGui::EndTable();
				};
			});
		}

		void CircleCollider2DInspector::SetSceneEntity(const SceneEntity& entity)
		{
			m_Context = entity;
		}

		ChainCollider2DInspector::ChainCollider2DInspector()
			:
			Inspectable("ChainCollider2DInspector")
		{
		}

		bool ChainCollider2DInspector::OnEditorRender()
		{
			return EditorHelper::DrawComponent<ChainCollider2DComponent>("Chain Collider2D", m_Context, [&](auto& component) {
				
				if (ImGui::BeginTable("##ChainCollider2DTable", 2, ImGuiTableFlags_SizingStretchProp))
				{
					UI::ScopedStyleStack style(true, ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 5.0f });
					UI::ScopedColorStack color(true,
						ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f },
						ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f },
						ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f }
					);
					const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

					UI::TableRow("Density",
						[]() { ImGui::Text("Density"); },
						[&]() { UI::ScopedTableColumnAutoWidth scoped(1, lineHeight);
					UI::FloatControl("##Density", "##DensityDrag", component.Density, 1.0f, 0.05f); }
					);
					UI::TableRow("Friction",
						[]() { ImGui::Text("Friction"); },
						[&]() { UI::ScopedTableColumnAutoWidth scoped(1, lineHeight);
					UI::FloatControl("##Friction", "##FrictionDrag", component.Friction, 1.0f, 0.05f); }
					);

					ImGui::EndTable();
				}
				UI::ContainerControl<1>("Points", component.Points, { "Position" }, m_SelectedPointIndex,
					[](glm::vec2& point, size_t& selectedIndex, size_t index) {

						bool selected = false;
						auto& colors = EditorLayer::GetData().Color;
						const std::string indexStr = std::to_string(index);
						const std::string countID = "##Point" + indexStr;

						UI::ScopedColorStack color(selectedIndex == index,
							ImGuiCol_FrameBg, colors[ED::ContainerSelectedItem]);


						UI::TableRow(indexStr.c_str(),
							[&]() {
								UI::ScopedTableColumnAutoWidth width(1);
								ImGui::DragFloat2(countID.c_str(), glm::value_ptr(point), sc_VSpeed);
								selected = ImGui::IsItemDeactivated(); 
						});
						if (selected)
							selectedIndex = index;
					});
			});
		}

		void ChainCollider2DInspector::SetSceneEntity(const SceneEntity& entity)
		{
			m_Context = entity;
		}
	}
}