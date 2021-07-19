#include "stdafx.h"
#include "AnimationEditor.h"

#include "XYZ/Animation/TransformTrack.h"
#include "XYZ/Scene/Components.h"

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
				m_Sequencer.AddSequencerItemType("Transform", { "Translation", "Rotation", "Scale" });
				m_Sequencer.Add(0);
			}
		}
		void AnimationEditor::OnImGuiRender(bool& open)
		{
			if (ImGui::Begin("Animation Editor", &open))
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
				ImSequencer::Sequencer(&m_Sequencer, &m_CurrentFrame, &m_Expanded, &m_SelectedEntry, &m_FirstFrame, ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE | ImSequencer::SEQUENCER_CHANGE_FRAME);
				// add a UI to edit that particular item
				if (m_SelectedEntry != -1)
				{
					AnimationSequencer::SequenceItem& item = m_Sequencer.m_Items[m_SelectedEntry];
					size_t index = 0;
					bool selected = item.LineEdit.GetSelectedIndex(index);
					if (selected && addKey)
					{
						item.LineEdit.AddPoint(index, ImVec2{ (float)m_CurrentFrame, 0.0f });
						auto transformTrack = m_Context->FindTrack<TransformTrack>();	
						auto& transform = transformTrack->GetSceneEntity().GetComponent<TransformComponent>();
					
						if (index == 0)
						{
							transformTrack->AddKeyFrame({ transform.Translation, m_CurrentFrame / 10.0f}, TransformTrack::PropertyType::Translation);
						}
						else if (index == 1)
						{
							transformTrack->AddKeyFrame({ transform.Rotation, m_CurrentFrame / 10.0f }, TransformTrack::PropertyType::Rotation);
						}
						else if (index == 2)
						{
							transformTrack->AddKeyFrame({ transform.Scale, m_CurrentFrame / 10.0f }, TransformTrack::PropertyType::Scale);
						}
						m_Context->UpdateLength();
					}
				}
			}
			ImGui::End();
		}
	}
}