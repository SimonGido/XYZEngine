#include "stdafx.h"
#include "InspectorPanel.h"

#include "XYZ/Renderer/Font.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Utils/FileSystem.h"
#include "XYZ/Utils/StringUtils.h"
#include "XYZ/Script/ScriptEngine.h"
#include "XYZ/Asset/AssetManager.h"

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
					Helper::DrawComponent<CameraComponent>("Camera", m_Context, [&](auto& component) {

						auto& camera = component.Camera;
						if (ImGui::Button("Projection"))
							ImGui::OpenPopup("Projection Type");

					
						if (ImGui::BeginPopup("Projection Type"))
						{
							if (ImGui::MenuItem("Perspective"))
							{
								camera.SetProjectionType(CameraProjectionType::Perspective);
								ImGui::CloseCurrentPopup();
							}
							if (ImGui::MenuItem("Orthographic"))
							{
								camera.SetProjectionType(CameraProjectionType::Orthographic);
								ImGui::CloseCurrentPopup();
							}
							ImGui::EndPopup();
						}

						ImGui::SameLine();
						if (camera.GetProjectionType() == CameraProjectionType::Orthographic)
						{
							ImGui::Text("Orthographic");
							CameraOrthographicProperties props = camera.GetOrthographicProperties();
							Helper::BeginColumns("Size");
							ImGui::InputFloat("##Size", &props.OrthographicSize);
							Helper::EndColumns();

							Helper::BeginColumns("Near");
							ImGui::InputFloat("##Near", &props.OrthographicNear);
							Helper::EndColumns();

							Helper::BeginColumns("Far");
							ImGui::InputFloat("##Far", &props.OrthographicFar);
							Helper::EndColumns();
							camera.SetOrthographic(props);
						}
						else
						{
							ImGui::Text("Perspective");
							CameraPerspectiveProperties props = camera.GetPerspectiveProperties();
							Helper::BeginColumns("FOV");
							ImGui::InputFloat("##FOV", &props.PerspectiveFOV);
							Helper::EndColumns();

							Helper::BeginColumns("Near");
							ImGui::InputFloat("##Near", &props.PerspectiveNear);
							Helper::EndColumns();

							Helper::BeginColumns("Far");
							ImGui::InputFloat("##Far", &props.PerspectiveFar);
							Helper::EndColumns();
							camera.SetPerspective(props);
						}
					});
					Helper::DrawComponent<TransformComponent>("Transform", m_Context, [&](auto& component) {
						
						Helper::DrawVec3Control("Translation", component.Translation);
						glm::vec3 rotation = glm::degrees(component.Rotation);
						Helper::DrawVec3Control("Rotation", rotation);
						component.Rotation = glm::radians(rotation);
						Helper::DrawVec3Control("Scale", component.Scale, 1.0f);
					});
					
					Helper::DrawComponent<SpriteRenderer>("Sprite Renderer", m_Context, [&](auto& component) {

						Helper::DrawColorControl("Color", component.Color);
						// Material
						{
							Helper::BeginColumns("Material");

							ImGui::PushItemWidth(150.0f);

							char materialPath[_MAX_PATH];
							memcpy(materialPath, component.Material->FileName.c_str(), component.Material->FileName.size());
							materialPath[component.Material->FileName.size()] = '\0';

							ImGui::InputText("##Material", materialPath, _MAX_PATH);
							ImGui::PopItemWidth();

							Helper::EndColumns();
						}
						/////////////////
						// SubTexture
						{
							Helper::BeginColumns("SubTexture");

							ImGui::PushItemWidth(150.0f);

							char subTexturePath[_MAX_PATH];
							memcpy(subTexturePath, component.SubTexture->FileName.c_str(), component.SubTexture->FileName.size());
							subTexturePath[component.SubTexture->FileName.size()] = '\0';

							ImGui::InputText("##SubTexture", subTexturePath, _MAX_PATH);
							ImGui::PopItemWidth();

							Helper::EndColumns();
						}
						/////////////////
						// Sort Layer
						{
							Helper::BeginColumns("Sort Layer");

							ImGui::PushItemWidth(75.0f);
							ImGui::InputInt("##Sort", (int*)&component.SortLayer);
							ImGui::PopItemWidth();

							Helper::EndColumns();
						}
						/////////////////
						// Visible
						{
							Helper::BeginColumns("Visible");
							ImGui::Checkbox("##Visible", &component.Visible);
							Helper::EndColumns();
						}
						/////////////////
						
					});

					Helper::DrawComponent<ScriptComponent>("Script", m_Context, [&](auto& component) {
						for (const PublicField& field : component.GetFields())
						{
							Helper::BeginColumns(field.GetName().c_str());
							std::string id = "##" + field.GetName();
							if (field.GetType() == PublicFieldType::Float)
							{
								float value = field.GetStoredValue<float>();
								if (ImGui::InputFloat(id.c_str(), &value))
								{
									field.SetStoredValue<float>(value);
								}
							}
							else if (field.GetType() == PublicFieldType::Int)
							{
								int32_t value = field.GetStoredValue<int32_t>();
								if (ImGui::InputInt(id.c_str(), &value))
								{
									field.SetStoredValue<int32_t>(value);
								}
							}
							else if (field.GetType() == PublicFieldType::UnsignedInt)
							{
								uint32_t value = field.GetStoredValue<uint32_t>();
								if (ImGui::InputInt(id.c_str(), (int*)&value))
								{
									field.SetStoredValue<uint32_t>(value);
								}
							}
							else if (field.GetType() == PublicFieldType::String)
							{
								char* value = field.GetStoredValue<char*>();
								size_t size = strlen(value);
								if (ImGui::InputText(id.c_str(), value, size))
								{
									field.SetStoredValue<char*>(value);
								}
								delete[]value;
							}
							else if (field.GetType() == PublicFieldType::Vec2)
							{
								glm::vec2 value = field.GetStoredValue<glm::vec2>();
								if (ImGui::InputFloat2(id.c_str(), glm::value_ptr(value)))
								{
									field.SetStoredValue<glm::vec2>(value);
								}
							}
							else if (field.GetType() == PublicFieldType::Vec3)
							{
								glm::vec3 value = field.GetStoredValue<glm::vec3>();
								if (ImGui::InputFloat3(id.c_str(), glm::value_ptr(value)))
								{
									field.SetStoredValue<glm::vec3>(value);
								}
							}
							else if (field.GetType() == PublicFieldType::Vec4)
							{
								glm::vec4 value = field.GetStoredValue<glm::vec4>();
								if (ImGui::InputFloat4(id.c_str(), glm::value_ptr(value)))
								{
									field.SetStoredValue<glm::vec4>(value);
								}
							}
							Helper::EndColumns();
						}
					});
					float addComponentButtonWidth = 200.0f;
					ImVec2 pos = ImGui::GetCursorPos();
					
					pos.x += (ImGui::GetWindowSize().x - addComponentButtonWidth) / 2.0f;
					pos.y += 25.0f;
					ImGui::SetCursorPos(pos);
					if (ImGui::Button("Add Component", { addComponentButtonWidth, 20.0f }))
						ImGui::OpenPopup("AddComponent");

					if (ImGui::BeginPopup("AddComponent"))
					{
						if (!m_Context.HasComponent<TransformComponent>())
						{
							if (ImGui::MenuItem("Transform Component"))
							{
								m_Context.EmplaceComponent<TransformComponent>();
								ImGui::CloseCurrentPopup();
							}
						}
						if (!m_Context.HasComponent<SpriteRenderer>())
						{
							if (ImGui::MenuItem("Sprite Renderer"))
							{
								m_Context.EmplaceComponent<SpriteRenderer>(
									m_DefaultMaterial,
									m_DefaultSubTexture,
									glm::vec4(1.0f),
									0,
									true
								);
								ImGui::CloseCurrentPopup();
							}
						}
						if (!m_Context.HasComponent<CameraComponent>())
						{
							if (ImGui::MenuItem("Camera Component"))
							{
								m_Context.EmplaceComponent<CameraComponent>();
								ImGui::CloseCurrentPopup();
							}
						}
						if (!m_Context.HasComponent<ScriptComponent>())
						{			
							if (ImGui::BeginMenu("Script Component"))
							{
								auto scripts = std::move(AssetManager::FindAssetsByType(AssetType::Script));
								for (auto& script : scripts)
								{
									if (ImGui::MenuItem(script->FileName.c_str()))
									{
										m_Context.EmplaceComponent<ScriptComponent>(
											script->FileName	
										);
										ScriptEngine::InitScriptEntity(m_Context);
										ScriptEngine::InstantiateEntityClass(m_Context);
										ImGui::CloseCurrentPopup();
									}
								}
								ImGui::EndMenu();
							}
						}
						ImGui::EndPopup();
					}			
				}
			}
			ImGui::End();		
		}
	

		InspectorPanel::InspectorPanel()
		{
			m_DefaultMaterial   = AssetManager::GetAsset<Material>(AssetManager::GetAssetHandle("Assets/Materials/Material.mat"));
			m_DefaultSubTexture = AssetManager::GetAsset<SubTexture>(AssetManager::GetAssetHandle("Assets/SubTextures/player.subtex"));
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