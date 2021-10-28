#include "stdafx.h"
#include "SceneEntityInspectorContext.h"

#include "XYZ/Renderer/Font.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Utils/FileSystem.h"
#include "XYZ/Utils/StringUtils.h"
#include "XYZ/Script/ScriptEngine.h"
#include "XYZ/Asset/AssetManager.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Utils/Math/Ray.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Editor/EditorHelper.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/perpendicular.hpp>

#include <typeinfo>

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
	}

	namespace Editor {
		SceneEntityInspectorContext::SceneEntityInspectorContext()
			:
			m_IconSize(35.0f)
		{
			m_DefaultMaterial = AssetManager::GetAsset<Material>(AssetManager::GetAssetHandle("Assets/Materials/Material.mat"));
			m_DefaultSubTexture = AssetManager::GetAsset<SubTexture>(AssetManager::GetAssetHandle("Assets/SubTextures/player.subtex"));
		}
		void SceneEntityInspectorContext::OnImGuiRender(Ref<EditorRenderer> renderer)
		{
			if (m_Context && m_Context.IsValid())
			{		
				for (auto it = m_InspectorEditables.begin(); it != m_InspectorEditables.end(); )
				{
					if ((*it)->OnEditorRender(renderer))
					{
						it = m_InspectorEditables.erase(it);
					}
					else
					{
						++it;
					}
				}
			

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
							m_InspectorEditables.push_back(&m_TransformInspector);
							m_TransformInspector.m_Context = m_Context;
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
							m_InspectorEditables.push_back(&m_SpriteRendererInspector);
							m_SpriteRendererInspector.m_Context = m_Context;
							ImGui::CloseCurrentPopup();
						}
					}
					if (!m_Context.HasComponent<CameraComponent>())
					{
						if (ImGui::MenuItem("Camera Component"))
						{
							m_Context.EmplaceComponent<CameraComponent>();
							m_InspectorEditables.push_back(&m_CameraInspector);
							m_CameraInspector.m_Context = m_Context;
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

									m_InspectorEditables.push_back(&m_ScriptComponentInspector);
									m_ScriptComponentInspector.m_Context = m_Context;
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
							m_InspectorEditables.push_back(&m_RigidBodyInspector);
							m_RigidBodyInspector.m_Context = m_Context;
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
							m_InspectorEditables.push_back(&m_BoxCollider2DInspector);
							m_BoxCollider2DInspector.m_Context = m_Context;
							ImGui::CloseCurrentPopup();
						}
						if (ImGui::MenuItem("Circle Collider2D"))
						{
							m_Context.EmplaceComponent<CircleCollider2DComponent>();
							m_InspectorEditables.push_back(&m_CircleCollider2DInspector);
							m_CircleCollider2DInspector.m_Context = m_Context;
							ImGui::CloseCurrentPopup();
						}
						if (ImGui::MenuItem("Chain Collider2D"))
						{
							m_Context.EmplaceComponent<ChainCollider2DComponent>();
							m_InspectorEditables.push_back(&m_ChainCollider2DInspector);
							m_ChainCollider2DInspector.m_Context = m_Context;
							ImGui::CloseCurrentPopup();
						}
					}
					if (!m_Context.HasComponent<PointLight2D>())
					{
						if (ImGui::MenuItem("Point Light2D"))
						{
							m_Context.EmplaceComponent<PointLight2D>();
							m_InspectorEditables.push_back(&m_PointLight2DInspector);
							m_PointLight2DInspector.m_Context = m_Context;
							ImGui::CloseCurrentPopup();
						}
					}
					if (!m_Context.HasComponent<SpotLight2D>())
					{
						if (ImGui::MenuItem("Spot Light2D"))
						{
							m_Context.EmplaceComponent<SpotLight2D>();
							m_InspectorEditables.push_back(&m_SpotLight2DInspector);
							m_SpotLight2DInspector.m_Context = m_Context;
							ImGui::CloseCurrentPopup();
						}
					}
					ImGui::EndPopup();
				}
			}
		}
		void SceneEntityInspectorContext::SetContext(SceneEntity context)
		{
			m_Context = context;
			m_InspectorEditables.clear();
			if (!m_Context.IsValid())
				return;

			if (m_Context.HasComponent<SceneTagComponent>())
			{
				m_InspectorEditables.push_back(&m_SceneTagInspector);
				m_SceneTagInspector.m_Context = context;
			}
			if (m_Context.HasComponent<TransformComponent>())
			{
				m_InspectorEditables.push_back(&m_TransformInspector);
				m_TransformInspector.m_Context = context;
			}
			if (m_Context.HasComponent<CameraComponent>())
			{
				m_InspectorEditables.push_back(&m_CameraInspector);
				m_CameraInspector.m_Context = context;
			}
			if (m_Context.HasComponent<PointLight2D>())
			{
				m_InspectorEditables.push_back(&m_PointLight2DInspector);
				m_PointLight2DInspector.m_Context = context;
			}
			if (m_Context.HasComponent<SpotLight2D>())
			{
				m_InspectorEditables.push_back(&m_SpotLight2DInspector);
				m_SpotLight2DInspector.m_Context = context;
			}
			if (m_Context.HasComponent<ScriptComponent>())
			{
				m_InspectorEditables.push_back(&m_ScriptComponentInspector);
				m_ScriptComponentInspector.m_Context = context;
			}
			if (m_Context.HasComponent<SpriteRenderer>())
			{
				m_InspectorEditables.push_back(&m_SpriteRendererInspector);
				m_SpriteRendererInspector.m_Context = context;
			}
			if (m_Context.HasComponent<RigidBody2DComponent>())
			{
				m_InspectorEditables.push_back(&m_RigidBodyInspector);
				m_RigidBodyInspector.m_Context = context;
			}
			if (m_Context.HasComponent<BoxCollider2DComponent>())
			{
				m_InspectorEditables.push_back(&m_BoxCollider2DInspector);
				m_BoxCollider2DInspector.m_Context = context;
			}
			if (m_Context.HasComponent<CircleCollider2DComponent>())
			{
				m_InspectorEditables.push_back(&m_CircleCollider2DInspector);
				m_CircleCollider2DInspector.m_Context = context;
			}
			if (m_Context.HasComponent<ChainCollider2DComponent>())
			{
				m_InspectorEditables.push_back(&m_ChainCollider2DInspector);
				m_ChainCollider2DInspector.m_Context = context;
			}
			if (m_Context.HasComponent<ParticleComponentGPU>())
			{
				m_InspectorEditables.push_back(&m_ParticleGPUInspector);
				m_ParticleGPUInspector.m_Context = context;
			}
			if (m_Context.HasComponent<ParticleComponentCPU>())
			{
				m_InspectorEditables.push_back(&m_ParticleCPUInspector);
				m_ParticleCPUInspector.m_Context = context;
			}
		}
	}
}