#include "stdafx.h"
#include "SceneEntityInspectorContext.h"

#include "XYZ/Renderer/Font.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Utils/FileSystem.h"
#include "XYZ/Utils/StringUtils.h"
#include "XYZ/Script/ScriptEngine.h"
#include "XYZ/Asset/AssetManager.h"
#include "XYZ/Renderer/Renderer2D.h"

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

		static void DrawVec3ControlRGB(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
		{
			ImGuiIO& io = ImGui::GetIO();
			auto boldFont = io.Fonts->Fonts[0];
			BeginColumns(label.c_str());

			ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 5.0f });

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f });

			DrawFloatControl("R", "##R", values.x, resetValue, columnWidth);
			ImGui::SameLine();
			DrawFloatControl("G", "##G", values.y, resetValue, columnWidth);
			ImGui::SameLine();
			DrawFloatControl("B", "##B", values.z, resetValue, columnWidth);
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
		SceneEntityInspectorContext::SceneEntityInspectorContext()
			:
			m_IconSize(35.0f)
		{
			m_DefaultMaterial = AssetManager::GetAsset<Material>(AssetManager::GetAssetHandle("Assets/Materials/Material.mat"));
			m_DefaultSubTexture = AssetManager::GetAsset<SubTexture>(AssetManager::GetAssetHandle("Assets/SubTextures/player.subtex"));
		}
		void SceneEntityInspectorContext::OnImGuiRender()
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
						if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
						{
							m_Dialog = Hook(&SceneEntityInspectorContext::selectMaterialDialog, this);
							m_DialogOpen = true;
						}
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
						if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
						{
							m_Dialog = Hook(&SceneEntityInspectorContext::selectSubTextureDialog, this);
							m_DialogOpen = true;
						}

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

				Helper::DrawComponent<PointLight2D>("Point Light2D", m_Context, [&](auto& component) {

					Helper::DrawVec3ControlRGB("Color", component.Color);
					Helper::BeginColumns("Radius");
					ImGui::DragFloat("##Radius", &component.Radius, 0.05f, 0.0f, 0.0f, "%.2f");
					Helper::EndColumns();

					Helper::BeginColumns("Intensity");
					ImGui::DragFloat("##Intensity", &component.Intensity, 0.05f, 0.0f, 0.0f, "%.2f");
					Helper::EndColumns();
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
				Helper::DrawComponent<RigidBody2DComponent>("Rigid Body2D", m_Context, [&](auto& component) {

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
				Helper::DrawComponent<BoxCollider2DComponent>("Box Collider2D", m_Context, [&](auto& component) {

					auto [translation, rotation, scale] = m_Context.GetComponent<TransformComponent>().GetWorldComponents();
					Renderer2D::SubmitLineQuad(
						translation - glm::vec3(component.Size.x / 2.0f, component.Size.y / 2.0f, 0.0f),
						component.Size,
						sc_ColliderColor
					);

					Helper::DrawVec2Control("Size", component.Size);

					Helper::BeginColumns("Density");
					ImGui::PushItemWidth(75.0f);
					ImGui::InputFloat("##Density", &component.Density);
					ImGui::PopItemWidth();
					Helper::EndColumns();


					Helper::BeginColumns("Friction");
					ImGui::PushItemWidth(75.0f);
					ImGui::InputFloat("##Friction", &component.Friction);
					ImGui::PopItemWidth();
					Helper::EndColumns();
				});

				Helper::DrawComponent<ChainCollider2DComponent>("Chain Collider2D", m_Context, [&](auto& component) {

					const TransformComponent& transform = m_Context.GetComponent<TransformComponent>();
					auto [translation, rotation, scale] = transform.GetWorldComponents();
					glm::vec3 p0(0.0f);
					glm::vec3 p1(0.0f);
					for (size_t i = 1; i < component.Points.size(); ++i)
					{
						p0 = { component.Points[i - 1].x, component.Points[i - 1].y, 0.0f };
						p1 = { component.Points[i].x, component.Points[i].y, 0.0f };
						p0 += translation;
						p1 += translation;
						Renderer2D::SubmitLine(p0, p1, sc_ColliderColor);
						Renderer2D::SubmitCircle(p0, 0.05f, 10, sc_ColliderColor);
					}
					Renderer2D::SubmitCircle(p1, 0.05f, 10, sc_ColliderColor);

					Helper::BeginColumns("Size");
					ImGui::PushItemWidth(75.0f);
					int size = component.Points.size();
					if (ImGui::InputInt("##Size", &size))
					{
						if (size >= 0 && (size_t)size != component.Points.size())
						{
							component.Points.resize(size);
						}
					}
					ImGui::PopItemWidth();
					Helper::EndColumns();


					//Helper::BeginColumns("Points");
					//ImGui::PushItemWidth(100.0f);
					uint32_t counter = 0;
					for (auto& point : component.Points)
					{
						Helper::DrawVec2Control(std::to_string(counter), point);
						//ImGui::PushID(counter);
						//
						//ImGui::InputFloat2("##Point", glm::value_ptr(point), 2);
						//ImGui::PopID();
						counter++;
					}
					//ImGui::PopItemWidth();
					//Helper::EndColumns();
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
					if (!m_Context.HasComponent<RigidBody2DComponent>())
					{
						if (ImGui::MenuItem("Rigid Body2D"))
						{
							m_Context.EmplaceComponent<RigidBody2DComponent>();
							ImGui::CloseCurrentPopup();
						}
					}
					if (!m_Context.HasComponent<BoxCollider2DComponent>()
						&& !m_Context.HasComponent<CircleCollider2DComponent>()
						&& !m_Context.HasComponent<ChainCollider2DComponent>())
					{
						if (ImGui::MenuItem("Box Collider2D"))
						{
							m_Context.EmplaceComponent<BoxCollider2DComponent>();
							ImGui::CloseCurrentPopup();
						}
						if (ImGui::MenuItem("Circle Collider2D"))
						{
							m_Context.EmplaceComponent<CircleCollider2DComponent>();
							ImGui::CloseCurrentPopup();
						}
						if (ImGui::MenuItem("Chain Collider2D"))
						{
							m_Context.EmplaceComponent<ChainCollider2DComponent>();
							ImGui::CloseCurrentPopup();
						}
					}
					if (!m_Context.HasComponent<PointLight2D>())
					{
						if (ImGui::MenuItem("Point Light2D"))
						{
							m_Context.EmplaceComponent<PointLight2D>();
							ImGui::CloseCurrentPopup();
						}
					}
					ImGui::EndPopup();
				}
			}
			if (m_DialogOpen && m_Dialog)
				m_Dialog();
		}
		void SceneEntityInspectorContext::SetContext(SceneEntity context)
		{		
			m_Context = context;			
		}
		void SceneEntityInspectorContext::selectSubTextureDialog()
		{
			int flags = ImGuiWindowFlags_NoDocking;
			if (ImGui::Begin("Select SubTexture", &m_DialogOpen, flags))
			{
				auto subTextures = std::move(AssetManager::FindAssetsByType(AssetType::SubTexture));
				for (const Ref<SubTexture>& subTexture : subTextures)
				{
					if (subTexture->IsLoaded)
					{
						const Ref<Texture>& texture = subTexture->GetTexture();
						const glm::vec4& texCoords = subTexture->GetTexCoords();
						if (ImGui::ImageButton((void*)(uint64_t)texture->GetRendererID(), { m_IconSize.x, m_IconSize.y }, { texCoords.x, texCoords.w }, { texCoords.z, texCoords.y }))
						{
							m_Context.GetComponent<SpriteRenderer>().SubTexture = subTexture;
							m_DialogOpen = false;
							break;
						}
					}
				}
			}
			ImGui::End();
		}
		void SceneEntityInspectorContext::selectMaterialDialog()
		{
			int flags = ImGuiWindowFlags_NoDocking;
			if (ImGui::Begin("Select Material", &m_DialogOpen, flags))
			{
				auto materials = std::move(AssetManager::FindAssetsByType(AssetType::Material));
				for (const Ref<Material>& material : materials)
				{
					if (material->IsLoaded)
					{
						if (ImGui::Button(material->FileName.c_str()))
						{
							m_Context.GetComponent<SpriteRenderer>().Material = material;
							m_DialogOpen = false;
							break;
						}
					}
				}
			}
			ImGui::End();
		}
	}
}