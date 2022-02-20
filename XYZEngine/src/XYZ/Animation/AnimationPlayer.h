#pragma once
#include "XYZ/Scene/SceneEntity.h"
#include "Animation.h"

namespace XYZ {
	class AnimationPlayer
	{
	public:
		AnimationPlayer() = default;
		AnimationPlayer(const SceneEntity& entity, const Ref<Animation>& animation);
		
		void Update(Timestep ts);
		void Reset();
		void Clear();
		void SetCurrentFrame(uint32_t frame);

		void Create(const SceneEntity& entity, const Ref<Animation>& animation);

		uint32_t GetCurrentFrame() const;
	private:
		enum TransformTrack { Translation, Rotation, Scale, NumTransformTracks };

		template <typename T>
		struct TrackView
		{
			SceneEntity		   Entity;
			AnimationTrack<T>* Track;
			size_t			   Key;
		};

		void updateVec3TrackView(TrackView<glm::vec3>& tv, glm::vec3& val, uint32_t frame);
	
	private:
		std::vector<TrackView<glm::vec3>> m_TransformTracks[NumTransformTracks];

		SceneEntity    m_Entity;
		Ref<Animation> m_Animation;
		float		   m_CurrentTime;
	};
}