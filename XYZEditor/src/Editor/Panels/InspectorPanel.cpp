#include "stdafx.h"
#include "InspectorPanel.h"

#include "XYZ/Core/Application.h"

#include "XYZ/Renderer/Font.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Renderer/Renderer2D.h"


#include "XYZ/Utils/FileSystem.h"
#include "XYZ/Utils/StringUtils.h"
#include "XYZ/Script/ScriptEngine.h"
#include "XYZ/Asset/AssetManager.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Scene/Prefab.h"

#include "Editor/Event/EditorEvents.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>

namespace XYZ {

	namespace Editor {
		InspectorPanel::InspectorPanel(std::string name)
			:
			EditorPanel(std::move(name))
		{
			m_DefaultMaterial = Renderer::GetDefaultResources().RendererAssets.at("QuadMaterial").As<MaterialAsset>();
			Ref<Texture2D> whiteTexture = Renderer::GetDefaultResources().RendererAssets.at("WhiteTexture").As<Texture2D>();
			m_DefaultSubTexture = Ref<SubTexture>::Create(whiteTexture);
		}
		void InspectorPanel::OnImGuiRender(bool& open)
		{
			if (ImGui::Begin("Inspector", &open))
			{
				if (m_State == State::Entity)
					drawSelectedEntity();
				else if (m_State == State::Asset)
					drawSelectedAsset();
			}
			ImGui::End();
		}
		void InspectorPanel::OnUpdate(Timestep ts)
		{
		}
		bool InspectorPanel::OnEvent(Event& e)
		{
			EventDispatcher dispatcher(e);
			dispatcher.Dispatch<EditorEvent>([&](EditorEvent& editorEvent) -> bool{
				if (editorEvent.GetEditorEventType() == EditorEventType::EntitySelected)
				{
					EntitySelectedEvent& entitySelectedEvent = (EntitySelectedEvent&)editorEvent;
					m_SelectedEntity = entitySelectedEvent.GetEntity();
					if (!m_SelectedEntity.IsValid())
					{
						m_State = State::None;
					}
					else
					{
						m_State = State::Entity;								
					}
					selectEditablesInUse();
				}
				else if (editorEvent.GetEditorEventType() == EditorEventType::AssetSelected)
				{
					AssetSelectedEvent& assetSelectedEvent = (AssetSelectedEvent&)editorEvent;
					m_SelectedAsset = assetSelectedEvent.GetAsset();					
					if (!m_SelectedAsset.Raw())
					{
						m_State = State::None;
					}
					else
					{
						m_State = State::Asset;
					}
					selectEditablesInUse();
				}
				return false;
			});
			return false;
		}
		void InspectorPanel::SetSceneContext(const Ref<Scene>& scene)
		{
			
		}
		void InspectorPanel::drawSelectedEntity()
		{
			for (auto it = m_InspectablesInUse.begin(); it !=  m_InspectablesInUse.end(); )
			{
				if ((*it)->OnEditorRender())
				{
					it = m_InspectablesInUse.erase(it);
				}
				else
				{
					++it;
				}
			}
			drawAddComponent();
		}
		void InspectorPanel::drawSelectedAsset()
		{
			for (auto& editable : m_InspectablesInUse)
				editable->OnEditorRender();
		}
		
		void InspectorPanel::drawAddComponent()
		{
			float addComponentButtonWidth = 200.0f;
			ImVec2 pos = ImGui::GetCursorPos();

			pos.x += (ImGui::GetWindowSize().x - addComponentButtonWidth) / 2.0f;
			pos.y += 25.0f;
			ImGui::SetCursorPos(pos);
			if (ImGui::Button("Add Component", { addComponentButtonWidth, 20.0f }))
				ImGui::OpenPopup("AddComponent");

			if (ImGui::BeginPopup("AddComponent"))
			{
				if (!m_SelectedEntity.HasComponent<TransformComponent>())
				{
					if (ImGui::MenuItem("Transform Component"))
					{
						m_SelectedEntity.EmplaceComponent<TransformComponent>();
						activateInspector<TransformComponent>();
						ImGui::CloseCurrentPopup();
					}
				}
				if (!m_SelectedEntity.HasComponent<SpriteRenderer>())
				{
					if (ImGui::MenuItem("Sprite Renderer"))
					{
						m_SelectedEntity.EmplaceComponent<SpriteRenderer>(
							m_DefaultMaterial,
							m_DefaultSubTexture,
							glm::vec4(1.0f),
							0,
							true
							);

						activateInspector<SpriteRenderer>();
						ImGui::CloseCurrentPopup();
					}
				}
				if (!m_SelectedEntity.HasComponent<CameraComponent>())
				{
					if (ImGui::MenuItem("Camera Component"))
					{
						m_SelectedEntity.EmplaceComponent<CameraComponent>();
						activateInspector<CameraComponent>();
						ImGui::CloseCurrentPopup();
					}
				}
				if (!m_SelectedEntity.HasComponent<ScriptComponent>())
				{
					if (ImGui::BeginMenu("Script Component"))
					{
						for (const auto& name : ScriptEngine::GetEntityClasses())
						{
							if (ImGui::MenuItem(name.c_str()))
							{
								m_SelectedEntity.EmplaceComponent<ScriptComponent>(name);						
								activateInspector<ScriptComponent>();
								ImGui::CloseCurrentPopup();
							}
						}
						ImGui::EndMenu();
					}
				}
				if (!m_SelectedEntity.HasComponent<RigidBody2DComponent>())
				{
					if (ImGui::MenuItem("Rigid Body2D"))
					{
						m_SelectedEntity.EmplaceComponent<RigidBody2DComponent>();
						activateInspector<RigidBody2DComponent>();
						ImGui::CloseCurrentPopup();
					}
				}
				if (!m_SelectedEntity.HasComponent<BoxCollider2DComponent>()
					&& !m_SelectedEntity.HasComponent<CircleCollider2DComponent>()
					&& !m_SelectedEntity.HasComponent<ChainCollider2DComponent>())
				{
					if (ImGui::MenuItem("Box Collider2D"))
					{
						m_SelectedEntity.EmplaceComponent<BoxCollider2DComponent>();
						activateInspector<BoxCollider2DComponent>();
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::MenuItem("Circle Collider2D"))
					{
						m_SelectedEntity.EmplaceComponent<CircleCollider2DComponent>();
						activateInspector<CircleCollider2DComponent>();
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::MenuItem("Chain Collider2D"))
					{
						m_SelectedEntity.EmplaceComponent<ChainCollider2DComponent>();
						activateInspector<ChainCollider2DComponent>();
						ImGui::CloseCurrentPopup();
					}
				}
				if (!m_SelectedEntity.HasComponent<PointLightComponent2D>())
				{
					if (ImGui::MenuItem("Point Light2D"))
					{
						m_SelectedEntity.EmplaceComponent<PointLightComponent2D>();
						activateInspector<PointLightComponent2D>();
						ImGui::CloseCurrentPopup();
					}
				}
				if (!m_SelectedEntity.HasComponent<SpotLightComponent2D>())
				{
					if (ImGui::MenuItem("Spot Light2D"))
					{
						m_SelectedEntity.EmplaceComponent<SpotLightComponent2D>();
						activateInspector<SpotLightComponent2D>();
						ImGui::CloseCurrentPopup();
					}
				}
				if (!m_SelectedEntity.HasComponent<PointLightComponent3D>())
				{
					if (ImGui::MenuItem("Point Light3D"))
					{
						m_SelectedEntity.EmplaceComponent<PointLightComponent3D>();
						activateInspector<PointLightComponent3D>();
						ImGui::CloseCurrentPopup();
					}
				}
				if (!m_SelectedEntity.HasComponent<AnimationComponent>())
				{
					if (ImGui::MenuItem("Animation Component"))
					{
						auto& component = m_SelectedEntity.EmplaceComponent<AnimationComponent>();
						component.Playing = true;
						
						activateInspector<AnimationComponent>();
						ImGui::CloseCurrentPopup();
					}
				}
				if (!m_SelectedEntity.HasComponent<MeshComponent>())
				{
					if (ImGui::MenuItem("Mesh Component"))
					{
						auto& component = m_SelectedEntity.EmplaceComponent<MeshComponent>();
						activateInspector<MeshComponent>();
						ImGui::CloseCurrentPopup();
					}
				}
				if (!m_SelectedEntity.HasComponent<ParticleComponent>())
				{
					if (ImGui::MenuItem("Particle Component"))
					{
						auto& component = m_SelectedEntity.EmplaceComponent<ParticleComponent>();
						activateInspectors<ParticleComponent, ParticleRenderer>();
						if (!m_SelectedEntity.HasComponent<ParticleRenderer>())
						{
							auto& particleRenderer = m_SelectedEntity.EmplaceComponent<ParticleRenderer>();
							particleRenderer.Mesh = AssetManager::GetAsset<Mesh>("Resources/Meshes/Quad.mesh");
							particleRenderer.MaterialAsset = AssetManager::GetAsset<MaterialAsset>("Resources/Materials/DefaultParticle.mat");
						}
					}
				}
				ImGui::EndPopup();
			}
		}

		void InspectorPanel::selectEditablesInUse()
		{
			m_InspectablesInUse.clear();
			if (m_State == State::None)
				return;

			if (m_SelectedAsset.Raw() && m_State == State::Asset)
			{
				if (m_SelectedAsset->GetAssetType() == AssetType::Material)
				{
					m_InspectablesInUse.push_back(&m_MaterialInspector);
					m_MaterialInspector.SetAsset(m_SelectedAsset);
				}
				else if (m_SelectedAsset->GetAssetType() == AssetType::AnimationController)
				{
					m_InspectablesInUse.push_back(&m_AnimationControllerInspector);
					m_AnimationControllerInspector.SetAsset(m_SelectedAsset);
				}
			}

			if (m_SelectedEntity && m_State == State::Entity)
			{
				activateInspectors<XYZ_COMPONENTS>();
			}
		}
	}
}