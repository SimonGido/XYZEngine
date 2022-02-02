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
			m_Context = context;
			m_Animation = m_Context->GetAnimation();
			m_AnimSelectedEntity = m_Context->GetSceneEntity();
			m_ClassMap.BuildMap(m_AnimSelectedEntity);
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
					if (m_AnimSelectedEntity.IsValid())
					{
						const ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

						keySelectionActions();
						UI::SplitterV(&m_SplitterWidth, "##PropertySection", "##TimelineSection",
							[&]() { propertySection(); },
							[&]() { timelineSection(); });
					}
				}
			}
			ImGui::End();
		}

		void AnimationEditor::drawEntityTree(const SceneEntity& entity)
		{
			const auto& tag = entity.GetComponent<SceneTagComponent>().Name;
			const auto& rel = entity.GetComponent<Relationship>();

			ImGuiTreeNodeFlags flags = (m_AnimSelectedEntity == entity ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
			flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

			const bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
			if (ImGui::IsItemClicked())
			{
				if (m_AnimSelectedEntity != entity)
				{
					m_AnimSelectedEntity = entity;
					onEntitySelected();
				}
			}

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
						if (ImGui::IsItemClicked())
						{
							if (m_AnimSelectedEntity != entity)
							{
								m_AnimSelectedEntity = entity;
								onEntitySelected();
							}
						}
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

		void AnimationEditor::onEntitySelected()
		{
			//auto addToSequencer = [&](const auto& props) {
			//	for (const auto& prop : props)
			//	{
			//		if (prop.GetPath() == m_AnimSelectedEntity.GetComponent<SceneTagComponent>().Name)
			//		{
			//			if (!m_Sequencer.ItemExists(prop.GetComponentName()))
			//				m_Sequencer.AddItem(prop.GetComponentName());
			//			
			//			const int itemIndex = m_Sequencer.GetItemIndex(prop.GetComponentName());
			//			m_Sequencer.AddLine(prop.GetComponentName(), prop.GetValueName());
			//			for (const auto& keyFrame : prop.GetKeyFrames())
			//				m_Sequencer.AddKey(itemIndex, static_cast<int>(keyFrame.Frame));
			//		}
			//	}
			//};
			//
			//addToSequencer(m_Animation->GetProperties<glm::vec4>());
			//addToSequencer(m_Animation->GetProperties<glm::vec3>());
			//addToSequencer(m_Animation->GetProperties<glm::vec2>());
			//addToSequencer(m_Animation->GetProperties<float>());
			//addToSequencer(m_Animation->GetProperties<void*>());
		}

		void AnimationEditor::propertySection()
		{
			UI::ScopedStyleStack style(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
			if (ButtonTransparent("Beginning", m_ButtonSize, ED::MediaBeginningIcon))
			{

			}
			ImGui::SameLine();
			if (ButtonTransparent("PrevKeyFrame", m_ButtonSize, ED::MediaNextIcon, true))
			{

			}
			ImGui::SameLine();			
			if (ButtonTransparent("Play", m_ButtonSize, ED::MediaPlayIcon))
			{
				m_Playing = !m_Playing;
			}
			ImGui::SameLine();
			if (ButtonTransparent("NextKeyFrame", m_ButtonSize, ED::MediaNextIcon))
			{

			}
			ImGui::SameLine();
			if (ButtonTransparent("End", m_ButtonSize, ED::MediaBeginningIcon, true))
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
						auto& val = reflClass.Get<variableIndex.value>(m_AnimSelectedEntity.GetComponentFromReflection(reflClass));
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
			ImGui::BeginNeo();
			std::string selectedEntity, selectedComponent, selectedValue;
			if (ImGui::BeginNeoSequencer("AnimationNeoSequencer", &m_CurrentFrame, &m_FrameMin, &m_FrameMax, &m_OffsetFrame, &m_Zoom))
			{ 
				for (auto& [entityName, componentData] : m_EntityPropertyMap)
				{
					ImGui::PushID(entityName.c_str());
					if (ImGui::BeginNeoGroup(entityName.c_str()))
					{
						for (auto& [componentName, propertyData] : componentData)
						{
							if (ImGui::BeginNeoGroup(componentName.c_str()))
							{
								for (auto& [prop, keyFrames, keyChangeFn] : propertyData)
								{
									if (ImGui::BeginNeoTimeline(prop->GetValueName().c_str(), keyFrames.data(), keyFrames.size(), keyChangeFn))
									{
										if (ImGui::IsEditingSelection())
										{
											XYZ_ASSERT(checkFramesValid(entityName, componentName, prop->GetValueName(), keyFrames), "");
										}
										if (ImGui::IsCurrentTimelineSelected())
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
				ImGui::EndNeoSequencer();
			}

			ImGui::EndNeo();


			if (ImGui::Button("Add Key") && !selectedEntity.empty())
			{
				handleAddKey(selectedEntity, selectedComponent, selectedValue);
			}
		}

		void AnimationEditor::handleEditKeyValues()
		{
	
		}
		
		void AnimationEditor::handleAddKey(std::string_view path, std::string_view componentName, std::string_view valueName)
		{
			SceneEntity entity = m_Context->GetSceneEntity();
			Entity childID = entity.GetComponent<Relationship>().FindByName(*entity.GetECS(), path);
			if (childID)
				entity = SceneEntity(childID, entity.GetScene());

			auto func = [&](auto classIndex, auto valueIndex) 
			{
				auto reflClass = ReflectedClasses::Get<classIndex.value>();
				auto& val = reflClass.Get<valueIndex.value>(entity.GetComponentFromReflection(reflClass));
				addKeyToProperty(path, componentName, valueName, m_CurrentFrame, val);
				m_EntityPropertyMap.BuildMap(m_Animation);
			};

			execFor(path, componentName, valueName, func);
		}


		void AnimationEditor::keySelectionActions()
		{
	
		}

		bool AnimationEditor::checkFramesValid(std::string_view path, std::string_view componentName, std::string_view valueName, const std::vector<ImNeoKeyFrame>& neoKeyFrames)
		{
			bool result = true;
			SceneEntity entity = m_Context->GetSceneEntity();
			Entity childID = entity.GetComponent<Relationship>().FindByName(*entity.GetECS(), path);
			if (childID)
				entity = SceneEntity(childID, entity.GetScene());
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
