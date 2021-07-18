#include "stdafx.h"
#include "AnimationEditor.h"

#include <imgui.h>
#include <ImSequencer.h>

namespace XYZ {
	namespace Editor {
		AnimationEditor::AnimationEditor()
		{
			m_Sequencer.m_FrameMin = 0;
			m_Sequencer.m_FrameMax = 100;
			
			m_Sequencer.AddItemType("Transform");
			m_Sequencer.AddItemType("Sprite");

			m_Sequencer.m_Items.push_back(AnimationSequencer::SequenceItem{ 0, 10, 30, false,{} });
			//m_Sequencer.m_Items.push_back(AnimationSequencer::SequenceItem{ 1, 20, 30, false,{} });			
		
			m_Sequencer.m_Items[0].LineEdit.AddLine("Translation");
			m_Sequencer.m_Items[0].LineEdit.AddLine("Rotation");
			m_Sequencer.m_Items[0].LineEdit.AddLine("Scale");

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
				if (ImGui::CollapsingHeader("Sequencer"))
				{
					// let's create the sequencer
					static int selectedEntry = -1;
					static int firstFrame = 0;
					static bool expanded = true;
					static int currentFrame = 100;

					ImGui::PushItemWidth(130);
					ImGui::InputInt("Frame Min", &m_Sequencer.m_FrameMin);
					ImGui::SameLine();
					ImGui::InputInt("Frame Max", &m_Sequencer.m_FrameMax);
					ImGui::SameLine();
					ImGui::InputInt("Frame ", &currentFrame);
					ImGui::SameLine();
					if (ImGui::Button("Add Key"))
					{
						//m_Sequencer.m_Items
					}
					ImGui::PopItemWidth();
					ImSequencer::Sequencer(&m_Sequencer, &currentFrame, &expanded, &selectedEntry, &firstFrame, ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE | ImSequencer::SEQUENCER_CHANGE_FRAME);
					// add a UI to edit that particular item
					if (selectedEntry != -1)
					{
						const AnimationSequencer::SequenceItem& item = m_Sequencer.m_Items[selectedEntry];
					}
				}

			}
			ImGui::End();
		}
	}
}