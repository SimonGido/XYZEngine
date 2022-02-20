#pragma once
#include "Editor/EditorPanel.h"
#include "EntityTrackMap.h"

#include "XYZ/Animation/Animation.h"
#include "XYZ/Animation/AnimationPlayer.h"

namespace XYZ {
	namespace Editor {
		class AnimationEditor : public EditorPanel
		{
		public:
			AnimationEditor(std::string name);

			virtual void OnImGuiRender(bool& open) override;
			virtual void OnUpdate(Timestep ts) override;
			virtual void SetSceneContext(const Ref<Scene>& scene) override;

			void SetContext(const Ref<Animation>& context);

		private:
			void propertySection();
			void timelineSection();

			template <typename T>
			void handleAddKey(SceneEntity entity, AnimationTrack<T>& track);

			template <typename T>
			AnimationTrack<T>* findTrack(const std::string& name, TrackType type);

			SceneEntity findEntity(const std::string& name);
		private:
			glm::vec2		   m_ButtonSize;
			SceneEntity		   m_Entity;
			Ref<Animation>	   m_Animation;
			Ref<Scene>		   m_Scene;

			AnimationPlayer	   m_Player;
			EntityTrackMap	   m_TrackMap;

			bool		m_Playing;
			float		m_SplitterWidth;

			uint32_t	m_CurrentFrame = 0;
			uint32_t	m_FrameMin = 0;
			uint32_t	m_FrameMax = 30;
			uint32_t	m_OffsetFrame = 0;
			float		m_Zoom = 1.0f;

		};
		template<typename T>
		inline void AnimationEditor::handleAddKey(SceneEntity entity, AnimationTrack<T>& track)
		{
			if (track.GetTrackType() == TrackType::Translation)
			{
				track.Keys.push_back({
					entity.GetComponent<TransformComponent>().Translation, m_CurrentFrame
				});
			}
			else if (track.GetTrackType() == TrackType::Rotation)
			{
				track.Keys.push_back({
					entity.GetComponent<TransformComponent>().Rotation, m_CurrentFrame
					});
			}
			else if (track.GetTrackType() == TrackType::Scale)
			{
				track.Keys.push_back({
					entity.GetComponent<TransformComponent>().Scale, m_CurrentFrame
				});
			}
		}
		template<typename T>
		inline AnimationTrack<T>* AnimationEditor::findTrack(const std::string& name, TrackType type)
		{
			auto& tracks = m_Animation->GetTracks<T>();
			for (auto& track : tracks)
			{
				if (track.GetName() == name && track.GetTrackType() == type)
					return &track;
			}
			return nullptr;
		}
	}
}