#include "stdafx.h"
#include "AnimationEditor.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Core/Input.h"
#include "XYZ/ImGui/ImGui.h"
#include "XYZ/Asset/AssetSerializer.h"


#include "Editor/EditorHelper.h"
#include "EditorLayer.h"

#include "XYZ/ImGui/NeoSequencer/imgui_neo_sequencer.h"

#include <imgui.h>

#include <numeric>
#include <algorithm>

namespace XYZ {
	namespace Editor {
		template <typename T>
		bool CompareKeyFrames(const std::vector<KeyFrame<T>>& keyFrames, const std::vector<ImNeoKeyFrame>& neoKeyFrames)
		{
			if (keyFrames.size() != neoKeyFrames.size())
				return false;
			for (size_t i = 0; i < keyFrames.size(); ++i)
			{
				if (keyFrames[i].Frame != neoKeyFrames[i].Frame)
					return false;
			}
			return true;
		}
		static bool EditValue(glm::vec4& value)
		{
			bool result = false;
			if (ImGui::InputFloat("X", &value.x))
				result = true;
			if (ImGui::InputFloat("Y", &value.y))
				result = true;
			if (ImGui::InputFloat("Z", &value.z))
				result = true;
			if (ImGui::InputFloat("W", &value.w))
				result = true;
			return result;
		}
		static bool EditValue(glm::vec3& value)
		{
			bool result = false;
			if (ImGui::InputFloat("X", &value.x))
				result = true;
			if (ImGui::InputFloat("Y", &value.y))
				result = true;
			if (ImGui::InputFloat("Z", &value.z))
				result = true;
			return result;
		}
		static bool EditValue(glm::vec2& value)
		{
			bool result = false;
			if (ImGui::InputFloat("X", &value.x))
				result = true;
			if (ImGui::InputFloat("Y", &value.y))
				result = true;
			return result;
		}

		static bool EditValue(float& value)
		{
			if (ImGui::InputFloat("Value", &value))
				return true;
			return false;
		}

		AnimationEditor::AnimationEditor(std::string name)
			:
			EditorPanel(std::move(name)),
			m_ButtonSize(25.0f),
			m_CurrentFrame(0),
			m_Playing(false),
			m_SplitterWidth(300.0f)
		{
		}
		void AnimationEditor::SetContext(const Ref<Animator>& context)
		{
			if (m_Context.Raw())
			{
				SceneEntity contextEntity = m_Context->GetSceneEntity();
				auto ecs = contextEntity.GetECS();
				if (ecs)
				{
					ecs->RemoveOnConstruction<&AnimationEditor::onEntityChanged>(this);
					ecs->RemoveOnDestruction<&AnimationEditor::onEntityChanged>(this);
				}
			}

			m_Context = context;
			m_Animation = m_Context->GetAnimation();
			m_FrameMax = m_Animation->GetNumFrames();

			SceneEntity contextEntity = m_Context->GetSceneEntity();
			m_ClassMap.BuildMap(contextEntity);

			auto ecs = contextEntity.GetECS();
			if (ecs)
			{
				ecs->AddOnConstruction<&AnimationEditor::onEntityChanged>(this);
				ecs->AddOnDestruction<&AnimationEditor::onEntityChanged>(this);
			}
		}
		void AnimationEditor::SetSceneContext(const Ref<Scene>& scene)
		{
			m_Scene = scene;
		}
		void AnimationEditor::OnUpdate(Timestep ts)
		{
			if (m_Playing && m_Context.Raw())
			{
				m_Animation->Update(ts);
				m_CurrentFrame = static_cast<int>(m_Animation->GetCurrentFrame());
				
				handleEditKeyValues();
			}
		}

		void AnimationEditor::OnImGuiRender(bool& open)
		{
			if (ImGui::Begin("Animation Editor", &open))
			{
				if (m_Context.Raw() && m_Scene.Raw())
				{
					const ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

					UI::SplitterV(&m_SplitterWidth, "##PropertySection", "##TimelineSection",
						[&]() { propertySection(); },
						[&]() { timelineSection(); });
				}
			}
			ImGui::End();
		}

		void AnimationEditor::drawEntityTree(const SceneEntity& entity)
		{
			const auto& tag = entity.GetComponent<SceneTagComponent>().Name;
			const auto& rel = entity.GetComponent<Relationship>();

			const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			const bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

			if (opened)
			{
				drawEntityTreeProperties(entity);
				if (rel.GetFirstChild())
					drawEntityTree(SceneEntity(rel.GetFirstChild(), m_Scene.Raw()));

				ImGui::TreePop();
			}

			if (rel.GetNextSibling())
				drawEntityTree(SceneEntity(rel.GetNextSibling(), m_Scene.Raw()));
		}

		void AnimationEditor::drawEntityTreeProperties(const SceneEntity& entity)
		{
			uint32_t currentFrame = static_cast<uint32_t>(m_CurrentFrame);
			auto drawProperties = [&](auto& props) {
				for (auto& prop : props)
				{
					if (prop.GetPath() == entity.GetComponent<SceneTagComponent>().Name)
					{			
						bool open = ImGui::TreeNodeEx(prop.GetValueName().c_str(), 0, prop.GetValueName().c_str());
						if (open)
						{
							auto val = prop.GetValue(currentFrame);
							if (EditValue(val))
							{
								if (prop.HasKeyAtFrame(currentFrame))
								{
									size_t key = prop.FindKey(currentFrame);
									prop.Keys[key].Value = val;
								}
								else
								{									
									prop.AddKeyFrame({ val, currentFrame });
									m_EntityPropertyMap.BuildMap(m_Animation);
								}
						
							}
							ImGui::TreePop();
						}
					}
				}
			};

			drawProperties(m_Animation->GetProperties<glm::vec4>());
			drawProperties(m_Animation->GetProperties<glm::vec3>());
			drawProperties(m_Animation->GetProperties<glm::vec2>());
			drawProperties(m_Animation->GetProperties<float>());
			//drawProperties(m_Animation->GetProperties<void*>());
		}

		void AnimationEditor::onEntityChanged(ECSManager& ecs, Entity entity)
		{
			if (m_Context.Raw())
			{
				SceneEntity contextEntity = m_Context->GetSceneEntity();
				if (entity = contextEntity)
				{
					m_ClassMap.BuildMap(contextEntity);
				}
			}
		}

		void AnimationEditor::propertySection()
		{
			UI::ScopedStyleStack style(ImGuiStyleVar_ItemSpacing, ImVec2(1.0f, 0.0f));

			if (ButtonTransparent("Beginning", m_ButtonSize + glm::vec2(10.0f, 0.0f), ED::MediaBeginningIcon))
			{

			}
			ImGui::SameLine();
			if (ButtonTransparent("PrevKeyFrame", m_ButtonSize , ED::MediaNextIcon, true))
			{

			}
			ImGui::SameLine();			
			if (ButtonTransparent("Play", m_ButtonSize, ED::MediaPlayIcon))
			{
				m_Playing = !m_Playing;
				m_Animation->Reset();
			}
			ImGui::SameLine();
			if (ButtonTransparent("NextKeyFrame", m_ButtonSize, ED::MediaNextIcon))
			{

			}
			ImGui::SameLine();
			if (ButtonTransparent("End", m_ButtonSize + glm::vec2(10.0f, 0.0f), ED::MediaBeginningIcon, true))
			{

			}


			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup("AddProperty");
			}
			if (ImGui::BeginPopup("AddProperty"))
			{
				std::string selectedEntity, selectedClass, selectedVariable;
				if (m_ClassMap.OnImGuiRender(selectedEntity, selectedClass, selectedVariable))
				{
					m_ClassMap.Execute(selectedClass, selectedVariable, [&](auto classIndex, auto variableIndex) 
					{
						auto reflClass = ReflectedClasses::Get<classIndex.value>();
						auto& val = reflClass.Get<variableIndex.value>(findEntity(selectedEntity).GetComponentFromReflection(reflClass));
						addReflectedProperty<variableIndex.value>(reflClass, val, selectedEntity, selectedVariable);
						m_Context->UpdateAnimationEntities();
						m_EntityPropertyMap.BuildMap(m_Animation);
					});
				}
				ImGui::EndPopup();
			}


			if (ImGui::BeginTable("##ControlTable", 2, ImGuiTableFlags_SizingFixedSame))
			{			
				UI::TableRow("FrameMax",
					[]() { ImGui::Text("Frame Max"); },
					[&]() 
					{
						if (ImGui::DragInt("##Frame Max", (int*)&m_FrameMax, 0.5f, m_FrameMin, INT_MAX, "%d"))
							m_Animation->SetNumFrames(m_FrameMax);
					});

				UI::TableRow("FrameMin",
					[]() { ImGui::Text("Frame Min"); },
					[&]()
					{
					ImGui::DragInt("##Frame Min", (int*)&m_FrameMin, 0.5f, 0, m_FrameMax, "%d");
					});

				UI::TableRow("Frame",
					[]() { ImGui::Text("Frame"); },
					[&]()
					{
						if (ImGui::DragInt("##Frame", (int*)&m_CurrentFrame, 0.5f, m_FrameMin, m_FrameMax, "%d"))
							m_Animation->SetCurrentFrame(m_CurrentFrame);
					});
				UI::TableRow("FPS",
					[]() { ImGui::Text("FPS"); },
					[&]()
					{
						int fps = static_cast<int>(m_Animation->GetFrequency());
						if (ImGui::DragInt("##FPS", &fps, 0.5f, 0, 0, "%d"))
							m_Animation->SetFrequency(static_cast<uint32_t>(fps));
					});

				
				ImGui::EndTable();
			}
			drawEntityTree(m_Context->GetSceneEntity());
		}

		

		void AnimationEditor::timelineSection()
		{		
			bool rebuildMap = false;
			if (!m_EntityPropertyMap.Empty())
			{
				std::string selectedEntity, selectedComponent, selectedValue;
				if (ImGui::BeginNeoSequencer("AnimationNeoSequencer", &m_CurrentFrame, &m_FrameMin, &m_FrameMax, &m_OffsetFrame, &m_Zoom))
				{
					bool deleted = false;

					for (auto& [entityName, entityData] : m_EntityPropertyMap)
					{
						ImGui::PushID(entityName.c_str());
						if (ImGui::BeginNeoGroup(entityName.c_str(), &entityData.Open))
						{
							for (auto& [componentName, propertyData] : entityData.Data)
							{
								if (ImGui::BeginNeoGroup(componentName.c_str(), &propertyData.Open))
								{
									for (auto& [prop, keyFrames, keyChangeFn, open] : propertyData.Properties)
									{
										if (ImGui::BeginNeoTimeline(prop->GetValueName().c_str(), keyFrames.data(), keyFrames.size(), keyChangeFn, &open))
										{
											if (ImGui::IsEditingSelection())
											{
												XYZ_ASSERT(checkFramesValid(entityName, componentName, prop->GetValueName(), keyFrames), "");
											}
											else if (Input::IsKeyPressed(KeyCode::KEY_DELETE))
											{
												auto& selection = ImGui::GetCurrentTimelineSelection();
												// Note: pointer passed to BeginNeoTimeline is no longer valid
												handleRemoveKeys(entityName, componentName, prop->GetValueName(), keyFrames, selection);
												deleted = true;
												rebuildMap = true;
											}
											else if (ImGui::IsCurrentTimelineSelected())
											{
												selectedEntity = entityName;
												selectedComponent = componentName;
												selectedValue = prop->GetValueName();
											}
											ImGui::EndNeoTimeLine();
										}
									}
									ImGui::EndNeoGroup();
								}
							}
							ImGui::EndNeoGroup();
						}
						ImGui::PopID();
					}
					if (deleted)
						ImGui::ClearSelection();

					ImGui::EndNeoSequencer();
				}


				if (ImGui::Button("Add Key") && !selectedEntity.empty())
				{
					handleAddKey(selectedEntity, selectedComponent, selectedValue);
				}
				if (rebuildMap)
				{
					m_EntityPropertyMap.BuildMap(m_Animation);
				}
			}
		}

		void AnimationEditor::handleEditKeyValues()
		{
	
		}
		
		void AnimationEditor::handleAddKey(std::string_view path, std::string_view componentName, std::string_view valueName)
		{
			SceneEntity entity = findEntity(path);
			auto func = [&](auto classIndex, auto valueIndex) 
			{
				auto reflClass = ReflectedClasses::Get<classIndex.value>();
				auto& val = reflClass.Get<valueIndex.value>(entity.GetComponentFromReflection(reflClass));
				addKeyToProperty(path, componentName, valueName, m_CurrentFrame, val);
				m_EntityPropertyMap.BuildMap(m_Animation);
			};

			execFor(path, componentName, valueName, func);
		}

		void AnimationEditor::handleRemoveKeys(std::string_view path, std::string_view componentName, std::string_view valueName, 
			std::vector<ImNeoKeyFrame>& keyFrames, const ImVector<uint32_t>& selection)
		{
			SceneEntity entity = findEntity(path);
			auto func = [&](auto classIndex, auto valueIndex, uint32_t keyIndex)
			{
				auto reflClass = ReflectedClasses::Get<classIndex.value>();
				auto& val = reflClass.Get<valueIndex.value>(entity.GetComponentFromReflection(reflClass));
				auto property = getProperty(val, path, componentName, valueName);
				auto& keys = property->Keys;
				keys.erase(keys.begin() + static_cast<size_t>(keyIndex));
			};

			for (int32_t i = selection.size() - 1; i >= 0; --i)
			{
				execFor(path, componentName, valueName, func, selection[i]);
			}
		}

		bool AnimationEditor::checkFramesValid(std::string_view path, std::string_view componentName, std::string_view valueName, const std::vector<ImNeoKeyFrame>& neoKeyFrames)
		{
			bool result = true;
			SceneEntity entity = findEntity(path);
			auto func = [&](auto classIndex, auto valueIndex)
			{
				auto reflClass = ReflectedClasses::Get<classIndex.value>();
				auto& val = reflClass.Get<valueIndex.value>(entity.GetComponentFromReflection(reflClass));
				auto property = getProperty(val, path, componentName, valueName);
				result &= std::is_sorted(property->Keys.begin(), property->Keys.end());
				result &= CompareKeyFrames(property->Keys, neoKeyFrames);
			};
			execFor(path, componentName, valueName, func);
			return result;
		}

		SceneEntity AnimationEditor::findEntity(std::string_view path) const
		{
			SceneEntity entity = m_Context->GetSceneEntity();
			Entity childID = entity.GetComponent<Relationship>().FindByName(*entity.GetECS(), path);
			if (childID)
				entity = SceneEntity(childID, entity.GetScene());
			return entity;
		}

		template <>
		bool AnimationEditor::editKeyValueSpecialized<glm::mat4>(uint32_t frame, glm::mat4& value, const std::string& valName)
		{
			return false;
		}
		template <>
		bool AnimationEditor::editKeyValueSpecialized<glm::vec4>(uint32_t frame, glm::vec4& value, const std::string& valName)
		{
			return false;
		}
		template <>
		bool AnimationEditor::editKeyValueSpecialized<glm::vec3>(uint32_t frame, glm::vec3& value, const std::string& valName)
		{
			bool result = false;
			ImGui::Text(valName.c_str());
			if (ImGui::InputFloat("X", &value.x))
				result = true;
			if (ImGui::InputFloat("Y", &value.y))
				result = true;
			if (ImGui::InputFloat("Z", &value.z))
				result = true;

			return result;
		}
		template <>
		bool AnimationEditor::editKeyValueSpecialized<glm::vec2>(uint32_t frame, glm::vec2& value, const std::string& valName)
		{
			return false;
		}
		template <>
		bool AnimationEditor::editKeyValueSpecialized<float>(uint32_t frame, float& value, const std::string& valName)
		{

			return false;
		}
		template <>
		bool AnimationEditor::editKeyValueSpecialized<uint32_t>(uint32_t frame, uint32_t& value, const std::string& valName)
		{

			return false;
		}
		template <>
		bool AnimationEditor::editKeyValueSpecialized<bool>(uint32_t frame, bool& value, const std::string& valName)
		{

			return false;
		}
		template <>
		bool AnimationEditor::editKeyValueSpecialized<void*>(uint32_t frame, void*& value, const std::string& valName)
		{

			return false;
		}
		template <>
		bool AnimationEditor::editKeyValueSpecialized<Ref<Material>>(uint32_t frame, Ref<Material>& value, const std::string& valName)
		{

			return false;
		}
		template <>
		bool AnimationEditor::editKeyValueSpecialized<Ref<SubTexture>>(uint32_t frame, Ref<SubTexture>& value, const std::string& valName)
		{

			return false;
		}
	}
}
