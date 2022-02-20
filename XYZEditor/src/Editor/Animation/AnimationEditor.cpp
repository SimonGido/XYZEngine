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
			EditorPanel(std::move(name)),
			m_ButtonSize(25.0f),
			m_CurrentFrame(0),
			m_Playing(false),
			m_SplitterWidth(300.0f)
		{
		}

		void AnimationEditor::SetContext(const Ref<Animation>& context)
		{
			m_Animation = context;
			m_FrameMax = m_Animation->NumFrames;
			m_TrackMap.BuildMap(m_Animation);
		}

		void AnimationEditor::SetSceneContext(const Ref<Scene>& scene)
		{
			m_Scene = scene;
		}
		void AnimationEditor::OnUpdate(Timestep ts)
		{
			if (m_Playing && m_Animation.Raw() && m_Entity.IsValid())
			{
				m_Player.Update(ts);
				m_CurrentFrame = static_cast<int>(m_Player.GetCurrentFrame());
			}
		}
		void AnimationEditor::OnImGuiRender(bool& open)
		{
			if (ImGui::Begin("Animation Editor", &open))
			{
				if (m_Scene.Raw())
				{
					if (m_Entity != m_Scene->GetSelectedEntity())
					{
						m_Entity = m_Scene->GetSelectedEntity();	
						m_Player.Create(m_Entity, m_Animation);
					}
					if (m_Animation.Raw() && m_Entity.IsValid())
					{
						const ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

						UI::SplitterV(&m_SplitterWidth, "##PropertySection", "##TimelineSection",
							[&]() { propertySection(); },
							[&]() { timelineSection(); });
					}
				}
			}
			ImGui::End();
		}
		void AnimationEditor::propertySection()
		{
			UI::ScopedStyleStack style(true, ImGuiStyleVar_ItemSpacing, ImVec2(1.0f, 0.0f));

			if (ButtonTransparent("Beginning", m_ButtonSize + glm::vec2(10.0f, 0.0f), ED::MediaBeginningIcon))
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
				m_Player.Create(m_Entity, m_Animation);
				m_Player.Reset();
			}
			ImGui::SameLine();
			if (ButtonTransparent("NextKeyFrame", m_ButtonSize, ED::MediaNextIcon))
			{

			}
			ImGui::SameLine();
			if (ButtonTransparent("End", m_ButtonSize + glm::vec2(10.0f, 0.0f), ED::MediaBeginningIcon, true))
			{

			}

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered())
			{
				ImGui::OpenPopup("AddProperty");
			}
			if (ImGui::BeginPopup("AddProperty"))
			{	
				const std::string entityName = m_Entity.GetComponent<SceneTagComponent>().Name;
				if (!m_Animation->HasTrack(entityName, TrackType::Translation))
				{
					if (ImGui::MenuItem("Translation"))
					{
						m_Animation->AddTrack(TrackType::Translation, entityName);
						m_TrackMap.BuildMap(m_Animation);
					}
				}
				if (!m_Animation->HasTrack(entityName, TrackType::Rotation))
				{
					if (ImGui::MenuItem("Rotation"))
					{
						m_Animation->AddTrack(TrackType::Rotation, entityName);
						m_TrackMap.BuildMap(m_Animation);
					}
				}
				if (!m_Animation->HasTrack(entityName, TrackType::Scale))
				{
					if (ImGui::MenuItem("Scale"))
					{
						m_Animation->AddTrack(TrackType::Scale, entityName);
						m_TrackMap.BuildMap(m_Animation);
					}
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
						m_Animation->NumFrames = m_FrameMax - m_FrameMin;
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
						m_Player.SetCurrentFrame(m_CurrentFrame);
				});
				UI::TableRow("FPS",
					[]() { ImGui::Text("FPS"); },
					[&]()
				{
					ImGui::DragInt("##FPS", (int*)&m_Animation->Frequency, 0.5f, 0, 1000, "%d");
				});


				ImGui::EndTable();
			}
		}
		void AnimationEditor::timelineSection()
		{
			std::string selectedTrackName;
			SceneEntity selectedEntity;
			TrackType	selectedTrackType = TrackType::None;
			TrackDataType selectedDataType = TrackDataType::None;

			bool deleted = false;
			if (ImGui::BeginNeoSequencer("AnimationNeoSequencer", &m_CurrentFrame, &m_FrameMin, &m_FrameMax, &m_OffsetFrame, &m_Zoom, ImVec2(0.0f, 0.0f), ImGuiNeoSequencerFlags_MouseScroll))
			{
				for (auto& [name, data] : m_TrackMap.DataMap)
				{
					if (ImGui::BeginNeoGroup(name.c_str(), &data.Open))
					{
						for (auto& [track, keyFrames, keyChangeFn, open] : data.Vec3Tracks)
						{
							const std::string trackName = TrackTypeToString(track->GetTrackType());
							if (ImGui::BeginNeoTimeline(trackName.c_str(), keyFrames.data(), keyFrames.size(), keyChangeFn, &open))
							{
								if (Input::IsKeyPressed(KeyCode::KEY_DELETE))
								{
									if (track->GetTrackDataType() == TrackDataType::Vec3)
									{
										auto& selection = ImGui::GetCurrentTimelineSelection();
										for (int32_t i = selection.size() - 1; i >= 0; --i)
											track->Keys.erase(track->Keys.begin() + selection[i]);
										deleted = true;
									}
								}
								else if (ImGui::IsCurrentTimelineSelected())
								{
									selectedTrackName = name;
									selectedTrackType = track->GetTrackType();
									selectedDataType = track->GetTrackDataType();
								}
								ImGui::EndNeoTimeLine();
							}
						}
						ImGui::EndNeoGroup();
					}
					if (deleted)
					{
						ImGui::ClearSelection();
						m_TrackMap.BuildMap(m_Animation);
					}
				}
				ImGui::EndNeoSequencer();
			}
			if (ImGui::Button("Add Key") && selectedTrackType != TrackType::None)
			{
				auto entity = findEntity(selectedTrackName);
				if (selectedDataType == TrackDataType::Vec3)
				{
					auto track = findTrack<glm::vec3>(selectedTrackName, selectedTrackType);			
					handleAddKey(entity, *track);
					std::sort(track->Keys.begin(), track->Keys.end());
				}
				m_TrackMap.BuildMap(m_Animation);
			}
		}
	
		SceneEntity AnimationEditor::findEntity(const std::string& name)
		{
			auto& ecs = *m_Entity.GetECS();
			auto entity = m_Entity.GetComponent<Relationship>().FindByName(ecs, name);
			if (!entity)
				entity = m_Entity;
			return { entity, m_Entity.GetScene() };
		}
	}
}