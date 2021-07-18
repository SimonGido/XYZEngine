#include "stdafx.h"
#include "AnimationEditor.h"

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
			
			m_Sequencer.AddItemType("Transform");
			m_Sequencer.AddItemType("Sprite");

			m_Sequencer.m_Items.push_back(AnimationSequencer::SequenceItem{ 0, 10, 30, false,{} });
			m_Sequencer.m_Items.push_back(AnimationSequencer::SequenceItem{ 1, 20, 30, false,{} });			
		
			m_Sequencer.m_Items[0].LineEdit.AddLine("Translation");
			m_Sequencer.m_Items[0].LineEdit.AddLine("Rotation");
			m_Sequencer.m_Items[0].LineEdit.AddLine("Scale");


			m_Sequencer.m_Items[1].LineEdit.AddLine("Sprite");


			m_Sequencer.m_Items[0].LineEdit.AddPoint(0, ImVec2{ 0.0f, 0.0f });
			m_Sequencer.m_Items[0].LineEdit.AddPoint(1, ImVec2{ 0.0f, 0.0f });
			m_Sequencer.m_Items[0].LineEdit.AddPoint(2, ImVec2{ 0.0f, 0.0f });
		}
		void AnimationEditor::SetContext(const Ref<Animation>& context)
		{
			m_Context = context;			
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
					}
				}
			}
			ImGui::End();
		}
	}
}