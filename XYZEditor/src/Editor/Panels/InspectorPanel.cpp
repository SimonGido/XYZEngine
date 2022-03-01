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
			m_DefaultMaterial = Renderer::GetDefaultResources().DefaultQuadMaterial;
			m_DefaultSubTexture = Ref<SubTexture>::Create(Renderer::GetDefaultResources().WhiteTexture);
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
			for (auto it = m_EditablesInUse.begin(); it !=  m_EditablesInUse.end(); )
			{
				if ((*it)->OnEditorRender())
				{
					it = m_EditablesInUse.erase(it);
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
			for (auto& editable : m_EditablesInUse)
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
						m_EditablesInUse.push_back(&m_TransformInspector);
						m_TransformInspector.SetSceneEntity(m_SelectedEntity);
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
						m_EditablesInUse.push_back(&m_SpriteRendererInspector);
						m_SpriteRendererInspector.SetSceneEntity(m_SelectedEntity);
						ImGui::CloseCurrentPopup();
					}
				}
				if (!m_SelectedEntity.HasComponent<CameraComponent>())
				{
					if (ImGui::MenuItem("Camera Component"))
					{
						m_SelectedEntity.EmplaceComponent<CameraComponent>();
						m_EditablesInUse.push_back(&m_CameraInspector);
						m_CameraInspector.SetSceneEntity(m_SelectedEntity);
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
							
								m_EditablesInUse.push_back(&m_ScriptComponentInspector);
								m_ScriptComponentInspector.SetSceneEntity(m_SelectedEntity);
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
						m_EditablesInUse.push_back(&m_RigidBodyInspector);
						m_RigidBodyInspector.SetSceneEntity(m_SelectedEntity);
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
						m_EditablesInUse.push_back(&m_BoxCollider2DInspector);
						m_BoxCollider2DInspector.SetSceneEntity(m_SelectedEntity);
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::MenuItem("Circle Collider2D"))
					{
						m_SelectedEntity.EmplaceComponent<CircleCollider2DComponent>();
						m_EditablesInUse.push_back(&m_CircleCollider2DInspector);
						m_CircleCollider2DInspector.SetSceneEntity(m_SelectedEntity);
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::MenuItem("Chain Collider2D"))
					{
						m_SelectedEntity.EmplaceComponent<ChainCollider2DComponent>();
						m_EditablesInUse.push_back(&m_ChainCollider2DInspector);
						m_ChainCollider2DInspector.SetSceneEntity(m_SelectedEntity);
						ImGui::CloseCurrentPopup();
					}
				}
				if (!m_SelectedEntity.HasComponent<PointLight2D>())
				{
					if (ImGui::MenuItem("Point Light2D"))
					{
						m_SelectedEntity.EmplaceComponent<PointLight2D>();
						m_EditablesInUse.push_back(&m_PointLight2DInspector);
						m_PointLight2DInspector.SetSceneEntity(m_SelectedEntity);
						ImGui::CloseCurrentPopup();
					}
				}
				if (!m_SelectedEntity.HasComponent<SpotLight2D>())
				{
					if (ImGui::MenuItem("Spot Light2D"))
					{
						m_SelectedEntity.EmplaceComponent<SpotLight2D>();
						m_EditablesInUse.push_back(&m_SpotLight2DInspector);
						m_SpotLight2DInspector.SetSceneEntity(m_SelectedEntity);
						ImGui::CloseCurrentPopup();
					}
				}
				if (!m_SelectedEntity.HasComponent<AnimationComponent>())
				{
					if (ImGui::MenuItem("Animation Component"))
					{
						auto& component = m_SelectedEntity.EmplaceComponent<AnimationComponent>();
						component.Playing = true;
						if (m_SelectedEntity.HasComponent<AnimatedMeshComponent>())
							component.BoneEntities = m_SelectedEntity.GetComponent<AnimatedMeshComponent>().BoneEntities;
					}
				}
				if (!m_SelectedEntity.HasComponent<ParticleComponent>())
				{
					if (ImGui::MenuItem("Particle Component"))
					{
						auto& component = m_SelectedEntity.EmplaceComponent<ParticleComponent>();
						m_ParticleInspector.SetSceneEntity(m_SelectedEntity);
						m_ParticleRendererInspector.SetSceneEntity(m_SelectedEntity);
						m_EditablesInUse.push_back(&m_ParticleInspector);
						m_EditablesInUse.push_back(&m_ParticleRendererInspector);
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
			m_EditablesInUse.clear();
			if (m_State == State::None)
				return;

			if (m_SelectedAsset.Raw() && m_State == State::Asset)
			{
				if (m_SelectedAsset->GetAssetType() == AssetType::Material)
				{
					m_EditablesInUse.push_back(&m_MaterialInspector);
					m_MaterialInspector.SetAsset(m_SelectedAsset);
				}
			}

			if (m_SelectedEntity && m_State == State::Entity)
			{
				if (m_SelectedEntity.HasComponent<SceneTagComponent>())
				{
					m_EditablesInUse.push_back(&m_SceneTagInspector);
				}
				if (m_SelectedEntity.HasComponent<TransformComponent>())
				{
					m_EditablesInUse.push_back(&m_TransformInspector);
				}
				if (m_SelectedEntity.HasComponent<CameraComponent>())
				{
					m_EditablesInUse.push_back(&m_CameraInspector);
				}
				if (m_SelectedEntity.HasComponent<PointLight2D>())
				{
					m_EditablesInUse.push_back(&m_PointLight2DInspector);
				}
				if (m_SelectedEntity.HasComponent<SpotLight2D>())
				{
					m_EditablesInUse.push_back(&m_SpotLight2DInspector);
				}
				if (m_SelectedEntity.HasComponent<ScriptComponent>())
				{
					m_EditablesInUse.push_back(&m_ScriptComponentInspector);
				}
				if (m_SelectedEntity.HasComponent<SpriteRenderer>())
				{
					m_EditablesInUse.push_back(&m_SpriteRendererInspector);
				}
				if (m_SelectedEntity.HasComponent<RigidBody2DComponent>())
				{
					m_EditablesInUse.push_back(&m_RigidBodyInspector);
				}
				if (m_SelectedEntity.HasComponent<BoxCollider2DComponent>())
				{
					m_EditablesInUse.push_back(&m_BoxCollider2DInspector);
				}
				if (m_SelectedEntity.HasComponent<CircleCollider2DComponent>())
				{
					m_EditablesInUse.push_back(&m_CircleCollider2DInspector);
				}
				if (m_SelectedEntity.HasComponent<ChainCollider2DComponent>())
				{
					m_EditablesInUse.push_back(&m_ChainCollider2DInspector);
				}
				if (m_SelectedEntity.HasComponent<ParticleComponent>())
				{
					m_EditablesInUse.push_back(&m_ParticleInspector);
				}
				if (m_SelectedEntity.HasComponent<ParticleRenderer>())
				{
					m_EditablesInUse.push_back(&m_ParticleRendererInspector);
				}
				if (m_SelectedEntity.HasComponent<MeshComponent>())
				{
					m_EditablesInUse.push_back(&m_MeshInspector);
				}
				if (m_SelectedEntity.HasComponent<AnimatedMeshComponent>())
				{
					m_EditablesInUse.push_back(&m_AnimatedMeshInspector);
				}
			}

			for (auto& [name, editable] : m_Editables)
			{
				if (editable->GetType() == Inspectable::Type::Entity)
					m_EditablesInUse.push_back(editable);
			}

			for (auto selected : m_EditablesInUse)
				selected->SetSceneEntity(m_SelectedEntity);
		}
	}
}