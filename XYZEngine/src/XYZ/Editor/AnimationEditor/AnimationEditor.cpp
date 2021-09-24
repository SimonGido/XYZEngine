#include "stdafx.h"
#include "AnimationEditor.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Core/Input.h"

#include <imgui.h>
#include <ImSequencer.h>

namespace XYZ {
	namespace Editor {

		namespace Helper {
			template <typename ...Args>
			static bool IntControl(const char* text, const char* id, int32_t& value, Args&& ...args)
			{
				ImGui::Text(text, std::forward<Args>(args)...);
				ImGui::SameLine();
				return ImGui::InputInt(id, &value);
			}		
	

			template <typename T>
			static void AddToClassMap(Reflection<T> refl, const SceneEntity& entity, const Ref<Animation>& anim, AnimationEditor::ClassMap& classMap)
			{
				if (entity.HasComponent<T>())
				{
					std::vector<std::string> tmpVariables;
					const char* className = refl.sc_ClassName;
					for (const auto& variable : refl.GetVariables())
					{
						if (!anim->PropertyHasVariable(className, variable.c_str()))
							tmpVariables.push_back(variable);
					}
					if (!tmpVariables.empty())
						classMap[refl.sc_ClassName].VariableNames = std::move(tmpVariables);
				}
			}

			template <typename ComponentType, typename T>
			void AddReflectedProperty(Reflection<ComponentType> refl, Ref<Animation> anim, const SceneEntity& entity, const T& val, const std::string& valName)
			{
				anim->AddProperty<ComponentType, T>(entity, valName);
			}
		}
		AnimationEditor::AnimationEditor()
			:
			m_SelectedEntry(-1),
			m_FirstFrame(0),
			m_CurrentFrame(0),
			m_Expanded(true),
			m_Playing(false)
		{
			m_Sequencer.m_FrameMin = 0;
			m_Sequencer.m_FrameMax = 100;
		}
		void AnimationEditor::SetContext(const Ref<Animation>& context)
		{
			m_Context = context;
		}
		void AnimationEditor::SetScene(const Ref<Scene>& scene)
		{
			m_Scene = scene;
		}
		void AnimationEditor::OnUpdate(Timestep ts)
		{
			if (m_Playing && m_Context.Raw())
			{
				m_Context->Update(ts);
			}
		}

		void AnimationEditor::OnImGuiRender(bool& open)
		{
			if (ImGui::Begin("Animation Editor", &open))
			{
				if (m_Context.Raw() && m_Scene.Raw())
				{
					SceneEntity selectedEntity = m_Scene->GetSelectedEntity();
					if (m_SelectedEntity != selectedEntity)
					{
						m_SelectedEntity = selectedEntity;
						buildClassMap(m_SelectedEntity);
					}
					if (selectedEntity.IsValid())
					{
						ImGui::PushItemWidth(130);
						if (ImGui::Button("Play"))
						{
							m_Playing = !m_Playing;
						}
						ImGui::SameLine();
						Helper::IntControl("Frame Min", "##Frame Min", m_Sequencer.m_FrameMin);

						ImGui::SameLine();
						if (Helper::IntControl("Frame Max", "##Frame Max", m_Sequencer.m_FrameMax))
						{
							m_Context->SetNumFrames(static_cast<uint32_t>(m_Sequencer.m_FrameMax));
						}
						ImGui::SameLine();
						if (Helper::IntControl("Frame", "##Frame", m_CurrentFrame))
						{
							m_Context->SetCurrentFrame(static_cast<uint32_t>(m_CurrentFrame));
						}
						ImGui::SameLine();

						if (ImGui::Button("Add Key"))
						{
							handleSelected();
						}
						ImGui::SameLine();
						if (ImGui::Button("Copy"))
						{
							m_Sequencer.Copy();
						}
						ImGui::SameLine();
						if (ImGui::Button("Paste"))
						{
							const auto& copy = m_Sequencer.GetCopy();
							AnimationSequencer::SequenceItem& item = m_Sequencer.m_Items[copy.ItemIndex];
							auto& seqItemType = m_Sequencer.m_SequencerItemTypes[item.Type];
							for (auto& it : copy.Points)
							{
								seqItemType.Callback(seqItemType.Entity, m_CurrentFrame + it.pointIndex, static_cast<uint32_t>(copy.ItemIndex));
							}
						}
						ImGui::PopItemWidth();

						ImGui::SameLine();
						if (ImGui::Button("Add Property"))
							ImGui::OpenPopup("AddProperty");

						if (ImGui::BeginPopup("AddProperty"))
						{
							auto [classIndex, variableIndex] = getClassAndVariable();
							Reflect::For([&](auto j) {
								if (j.value == classIndex)
								{
									auto reflClass = ReflectedClasses::Get<j.value>();
									Reflect::For([&](auto i) {
										if (i.value == variableIndex)
										{
											auto& val = reflClass.Get<i.value>(selectedEntity.GetComponentFromReflection(reflClass));
											Helper::AddReflectedProperty(reflClass, m_Context, selectedEntity, val,
												reflClass.GetVariables()[i.value]
											);
										}
									}, std::make_index_sequence<reflClass.sc_NumVariables>());
								}
							}, std::make_index_sequence<ReflectedClasses::sc_NumClasses>());

							ImGui::EndPopup();
						}

						ImSequencer::Sequencer(&m_Sequencer, &m_CurrentFrame, &m_Expanded, &m_SelectedEntry, &m_FirstFrame, ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_CHANGE_FRAME);

						if (Input::IsKeyPressed(KeyCode::KEY_DELETE))
						{
							const auto& selection = m_Sequencer.GetSelection();
							AnimationSequencer::SequenceItem& item = m_Sequencer.m_Items[selection.ItemIndex];
							auto& seqItemType = m_Sequencer.m_SequencerItemTypes[item.Type];
							size_t index = 0;
							bool selected = item.LineEdit.GetSelectedIndex(index);
							//if (selected
							//	&& seqItemType.Name == "Transform"
							//	&& seqItemType.Entity == selectedEntity)
							//{
							//	const TransformTrack::PropertyType type = static_cast<TransformTrack::PropertyType>(index);
							//	Ref<TransformTrack> track = m_Context->FindTrack<TransformTrack>(selectedEntity);
							//
							//	for (auto& point : selection.Points)
							//	{
							//		const auto& line = item.LineEdit.GetLines()[point.curveIndex];
							//		uint32_t frame = static_cast<uint32_t>(line.Points[point.pointIndex].x);
							//		track->RemoveKeyFrame(frame, type);
							//	}
							//}
							//m_Sequencer.DeleteSelectedPoints();
						}
						
					}
				}
			}
			ImGui::End();
		}
		void AnimationEditor::handleSelected()
		{
			if (m_SelectedEntry != -1)
			{
				AnimationSequencer::SequenceItem& item = m_Sequencer.m_Items[m_SelectedEntry];
				auto& seqItemType = m_Sequencer.m_SequencerItemTypes[item.Type];
				seqItemType.Callback(seqItemType.Entity, m_CurrentFrame, static_cast<uint32_t>(m_SelectedEntry));
				m_Sequencer.ClearSelection();
			}
		}

		void AnimationEditor::buildClassMap(const SceneEntity& entity)
		{
			m_ClassMap.clear();
			if (entity)
			{
				Reflect::For([&](auto j) {
					auto reflClass = ReflectedClasses::Get<j.value>();
					Helper::AddToClassMap(reflClass, entity, m_Context, m_ClassMap);

				}, std::make_index_sequence<ReflectedClasses::sc_NumClasses>());
			}
		}
		std::pair<int32_t, int32_t> AnimationEditor::getClassAndVariable()
		{
			for (auto& [className, classData] : m_ClassMap)
			{
				if (ImGui::BeginMenu(className.c_str()))
				{
					int32_t variableCounter = 0;
					auto& variables = classData.VariableNames;
					for (auto it = variables.begin(); it != variables.end(); ++it)
					{
						if (ImGui::MenuItem(it->c_str()))
						{
							variables.erase(it);
							int32_t classIndex = -1;
							for (const auto& it : ReflectedClasses::GetClasses())
							{
								if (it == className)
									return { classIndex, variableCounter };
								classIndex++;
							}				
						}
						variableCounter++;
					}
					ImGui::EndMenu();
				}
			}
			return std::pair<int32_t, int32_t>(-1, -1);
		}
	}
}