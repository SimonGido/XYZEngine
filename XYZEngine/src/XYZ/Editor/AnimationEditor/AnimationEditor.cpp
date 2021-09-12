#include "stdafx.h"
#include "AnimationEditor.h"

#include "XYZ/Animation/TransformTrack.h"
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
			//auto transformTrack = m_Context->FindTrack<TransformTrack>();
			//if (transformTrack.Raw())
			//{
			//	addTransformTrack();
			//	m_Sequencer.Add(m_Sequencer.GetItemTypeCount() - 1);
			//}
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
							if (!m_Context->FindTrack<TransformTrack>(selectedEntity).Raw())
							{
								if (ImGui::MenuItem("Transform"))
								{
									m_Context->CreateTrack<TransformTrack>(selectedEntity);
									addTransformTrack(selectedEntity);
									m_Sequencer.Add(m_Sequencer.GetItemTypeCount() - 1);
									ImGui::CloseCurrentPopup();
								}
							}
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
							if (selected
								&& seqItemType.Name == "Transform"
								&& seqItemType.Entity == selectedEntity)
							{
								const TransformTrack::PropertyType type = static_cast<TransformTrack::PropertyType>(index);
								Ref<TransformTrack> track = m_Context->FindTrack<TransformTrack>(selectedEntity);

								for (auto& point : selection.Points)
								{
									const auto& line = item.LineEdit.GetLines()[point.curveIndex];
									uint32_t frame = static_cast<uint32_t>(line.Points[point.pointIndex].x);
									track->RemoveKeyFrame(frame, type);
								}
							}
							m_Sequencer.DeleteSelectedPoints();
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
		void AnimationEditor::addTransformTrack(const SceneEntity& entity)
		{
			m_Sequencer.AddSequencerItemType("Transform", entity, { "Translation", "Rotation", "Scale" }, [&](const SceneEntity& targetEntity, uint32_t frame, uint32_t itemIndex) {
				AnimationSequencer::SequenceItem& item = m_Sequencer.m_Items[itemIndex];
				size_t index = 0;
				bool selected = item.LineEdit.GetSelectedIndex(index);
				
				if (targetEntity.IsValid())
				{
					if (selected)
					{
						item.LineEdit.AddPoint(index, ImVec2{ (float)frame, 0.0f });
						auto transformTrack = m_Context->FindTrack<TransformTrack>(targetEntity);
						auto& transform = transformTrack->GetSceneEntity().GetComponent<TransformComponent>();
						uint32_t currentFrame = static_cast<uint32_t>(frame);
						const TransformTrack::PropertyType type = static_cast<TransformTrack::PropertyType>(index);
						switch (type)
						{
						case XYZ::TransformTrack::PropertyType::Translation:
							transformTrack->AddKeyFrame({ transform.Translation, currentFrame }, type);
							break;
						case XYZ::TransformTrack::PropertyType::Rotation:
							transformTrack->AddKeyFrame({ transform.Rotation, currentFrame }, type);
							break;
						case XYZ::TransformTrack::PropertyType::Scale:
							transformTrack->AddKeyFrame({ transform.Scale, currentFrame }, type);
							break;
						default:
							break;
						}
						m_Context->UpdateLength();
					}
				}
			});
		}
	}
}