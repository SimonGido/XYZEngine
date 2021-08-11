#include "stdafx.h"
#include "AnimationEditor.h"

#include "XYZ/Animation/TransformTrack.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Core/Input.h"

#include <imgui.h>
#include <ImSequencer.h>

namespace XYZ {
	namespace Editor {

		AnimationEditor::AnimationEditor()
			:
			m_SelectedEntry(-1),
			m_FirstFrame(0),
			m_CurrentFrame(0),
			m_Expanded(true)
		{
			m_Sequencer.m_FrameMin = 0;
			m_Sequencer.m_FrameMax = 100;
		}
		void AnimationEditor::SetContext(const Ref<Animation>& context)
		{
			m_Context = context;		
			auto transformTrack = m_Context->FindTrack<TransformTrack>();
			if (transformTrack.Raw())
			{
				addTransformTrack();
				m_Sequencer.Add(m_Sequencer.GetItemTypeCount() - 1);
			}
		}
		void AnimationEditor::OnImGuiRender(bool& open)
		{
			if (ImGui::Begin("Animation Editor", &open))
			{
				if (m_Context.Raw())
				{
					ImGui::PushItemWidth(130);
					ImGui::InputInt("Frame Min", &m_Sequencer.m_FrameMin);
					ImGui::SameLine();
					ImGui::InputInt("Frame Max", &m_Sequencer.m_FrameMax);
					ImGui::SameLine();
					ImGui::InputInt("Frame ", &m_CurrentFrame);
					ImGui::SameLine();

					bool addKey = false;
					if (ImGui::Button("Add Key"))
					{
						addKey = true;
					}
					ImGui::PopItemWidth();

					ImGui::SameLine();
					if (ImGui::Button("Add Property"))
						ImGui::OpenPopup("AddProperty");

					if (ImGui::BeginPopup("AddProperty"))
					{
						if (!m_Context->FindTrack<TransformTrack>().Raw())
						{
							if (ImGui::MenuItem("Transform"))
							{
								m_Context->CreateTrack<TransformTrack>();
								addTransformTrack();
								m_Sequencer.Add(m_Sequencer.GetItemTypeCount() - 1);
								ImGui::CloseCurrentPopup();
							}
						}
						ImGui::EndPopup();
					}

					ImSequencer::Sequencer(&m_Sequencer, &m_CurrentFrame, &m_Expanded, &m_SelectedEntry, &m_FirstFrame, ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_CHANGE_FRAME);
					if (Input::IsKeyPressed(KeyCode::KEY_DELETE))
					{
						m_Sequencer.DeleteSelectedPoints();
					}
					handleSelected(addKey);
				}
			}
			ImGui::End();
		}
		void AnimationEditor::handleSelected(bool addKey)
		{
			if (m_SelectedEntry != -1 && addKey)
			{
				AnimationSequencer::SequenceItem& item = m_Sequencer.m_Items[m_SelectedEntry];
				m_Sequencer.m_SequencerItemTypes[item.Type].Callback();
			}
		}
		void AnimationEditor::addTransformTrack()
		{
			m_Sequencer.AddSequencerItemType("Transform", { "Translation", "Rotation", "Scale" }, [&]() {
				AnimationSequencer::SequenceItem& item = m_Sequencer.m_Items[m_SelectedEntry];
				size_t index = 0;
				bool selected = item.LineEdit.GetSelectedIndex(index);
				if (selected)
				{
					item.LineEdit.AddPoint(index, ImVec2{ (float)m_CurrentFrame, 0.0f });
					auto transformTrack = m_Context->FindTrack<TransformTrack>();
					auto& transform = transformTrack->GetSceneEntity().GetComponent<TransformComponent>();
					if (index == 0)
						transformTrack->AddKeyFrame({ transform.Translation, m_CurrentFrame / 10.0f }, TransformTrack::PropertyType::Translation);
					else if (index == 1)
						transformTrack->AddKeyFrame({ transform.Rotation, m_CurrentFrame / 10.0f }, TransformTrack::PropertyType::Rotation);
					else if (index == 2)
						transformTrack->AddKeyFrame({ transform.Scale, m_CurrentFrame / 10.0f }, TransformTrack::PropertyType::Scale);
					m_Context->UpdateLength();
				}
			});
		}
	}
}