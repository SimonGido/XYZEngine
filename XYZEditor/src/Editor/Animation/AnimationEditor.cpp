#include "stdafx.h"
#include "AnimationEditor.h"

#include "XYZ/Asset/AssetSerializer.h"

#include "Editor/EditorHelper.h"
#include "EditorLayer.h"


#include "XYZ/ImGui/NeoSequencer/imgui_neo_sequencer.h"
#include <imgui.h>


namespace XYZ {
	namespace Editor {
		AnimationEditor::AnimationEditor(std::string name)
			:
			EditorPanel(std::move(name))
		{
		}

		

		void AnimationEditor::SetSceneContext(const Ref<Scene>& scene)
		{
			
		}
		void AnimationEditor::OnUpdate(Timestep ts)
		{
			
		}
		void AnimationEditor::OnImGuiRender(bool& open)
		{
			if (ImGui::Begin("Animation Editor", &open))
			{
				
			}
			ImGui::End();
		}
		void AnimationEditor::propertySection()
		{

		}
		void AnimationEditor::timelineSection()
		{
			//std::string selectedTrackName;
			//SceneEntity selectedEntity;
			//TrackType	selectedTrackType = TrackType::None;
			//TrackDataType selectedDataType = TrackDataType::None;
			//
			//bool deleted = false;
			//if (ImGui::BeginNeoSequencer("AnimationNeoSequencer", &m_CurrentFrame, &m_FrameMin, &m_FrameMax, &m_OffsetFrame, &m_Zoom, ImVec2(0.0f, 0.0f), ImGuiNeoSequencerFlags_MouseScroll))
			//{
			//	for (auto& [name, data] : m_TrackMap.DataMap)
			//	{
			//		ImGui::PushID(name.c_str());
			//		if (ImGui::BeginNeoGroup(name.c_str(), &data.Open))
			//		{
			//			for (auto& [track, keyFrames, keyChangeFn, open] : data.Vec3Tracks)
			//			{
			//				const std::string trackName = TrackTypeToString(track->GetTrackType());
			//				if (ImGui::BeginNeoTimeline(trackName.c_str(), keyFrames.data(), keyFrames.size(), keyChangeFn, &open))
			//				{
			//					if (Input::IsKeyPressed(KeyCode::KEY_DELETE))
			//					{
			//						if (track->GetTrackDataType() == TrackDataType::Vec3)
			//						{
			//							auto& selection = ImGui::GetCurrentTimelineSelection();
			//							for (int32_t i = selection.size() - 1; i >= 0; --i)
			//								track->Keys.erase(track->Keys.begin() + selection[i]);
			//							deleted = true;
			//						}
			//					}
			//					else if (ImGui::IsCurrentTimelineSelected())
			//					{
			//						selectedTrackName = name;
			//						selectedTrackType = track->GetTrackType();
			//						selectedDataType = track->GetTrackDataType();
			//					}
			//					ImGui::EndNeoTimeLine();
			//				}
			//			}
			//			ImGui::EndNeoGroup();
			//		}
			//		if (deleted)
			//		{
			//			ImGui::ClearSelection();
			//			m_TrackMap.BuildMap(m_Animation);
			//		}
			//	}
			//	ImGui::EndNeoSequencer();
			//}
			//if (ImGui::Button("Add Key") && selectedTrackType != TrackType::None)
			//{
			//	auto entity = findEntity(selectedTrackName);
			//	if (selectedDataType == TrackDataType::Vec3)
			//	{
			//		auto track = findTrack<glm::vec3>(selectedTrackName, selectedTrackType);			
			//		handleAddKey(entity, *track);
			//		std::sort(track->Keys.begin(), track->Keys.end());
			//	}
			//	m_TrackMap.BuildMap(m_Animation);
			//}
		}

	}
}