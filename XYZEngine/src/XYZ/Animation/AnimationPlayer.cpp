#include "stdafx.h"
#include "AnimationPlayer.h"

#include "XYZ/Scene/Components.h"

namespace XYZ {
	AnimationPlayer::AnimationPlayer(const SceneEntity& entity, const Ref<Animation>& animation)
		:
		m_Entity(entity),
		m_Animation(animation)
	{
		Create(entity, animation);
	}

	void AnimationPlayer::Update(Timestep ts)
	{
		uint32_t frame = GetCurrentFrame();
		if (frame >= m_Animation->NumFrames)
		{
			if (m_Animation->Repeat)
				Reset();
			return;
		}

		for (auto& tv : m_TransformTracks[Translation])
		{
			TransformComponent& transform = tv.Entity.GetComponent<TransformComponent>();
			updateVec3TrackView(tv, transform.Translation, frame);
		}
		for (auto& tv : m_TransformTracks[Rotation])
		{
			TransformComponent& transform = tv.Entity.GetComponent<TransformComponent>();
			updateVec3TrackView(tv, transform.Rotation, frame);
		}
		for (auto& tv : m_TransformTracks[Scale])
		{
			TransformComponent& transform = tv.Entity.GetComponent<TransformComponent>();
			updateVec3TrackView(tv, transform.Scale, frame);
		}
		m_CurrentTime += ts;
	}

	void AnimationPlayer::Reset()
	{
		m_CurrentTime = 0.0f;
		for (size_t i = 0; i < NumTransformTracks; ++i)
		{
			for (auto& track : m_TransformTracks[i])
				track.Key = 0;
		}
	}
	void AnimationPlayer::Clear()
	{
		m_CurrentTime = 0.0f;
		for (size_t i = 0; i < NumTransformTracks; ++i)
		{
			m_TransformTracks[i].clear();
		}
	}
	void AnimationPlayer::SetCurrentFrame(uint32_t frame)
	{
		XYZ_ASSERT(m_Animation.Raw(), "");
		m_CurrentTime = frame * m_Animation->FrameLength();
		for (auto& tv : m_TransformTracks[Translation])
		{
			tv.Key = tv.Track->GetKey(frame);
		}
		for (auto& tv : m_TransformTracks[Rotation])
		{
			tv.Key = tv.Track->GetKey(frame);
		}
		for (auto& tv : m_TransformTracks[Scale])
		{
			tv.Key = tv.Track->GetKey(frame);
		}
	}
	void AnimationPlayer::Create(const SceneEntity& entity, const Ref<Animation>& animation)
	{
		Clear();
		m_Entity = entity;
		m_Animation = animation;
		if (!m_Entity.IsValid() || !m_Animation.Raw())
			return;

		auto& ecs = *m_Entity.GetECS();
		auto tree = m_Entity.GetComponent<Relationship>().GetTree(ecs);
		tree.push_back(m_Entity);
		for (auto child : tree)
		{
			for (auto& track : m_Animation->Vec3Tracks)
			{
				std::string name = ecs.GetComponent<SceneTagComponent>(child).Name;
				if (name == track.GetName())
				{
					TrackView<glm::vec3> tr{ SceneEntity(child, m_Entity.GetScene()), &track, 0 };
					if (track.GetTrackType() == TrackType::Translation)
					{
						m_TransformTracks[Translation].push_back(tr);
					}
					else if (track.GetTrackType() == TrackType::Rotation)
					{
						m_TransformTracks[Rotation].push_back(tr);
					}
					else if (track.GetTrackType() == TrackType::Scale)
					{
						m_TransformTracks[Scale].push_back(tr);
					}
				}
			}
		}
	}
	uint32_t AnimationPlayer::GetCurrentFrame() const
	{
		return static_cast<uint32_t>(std::floor(m_CurrentTime / m_Animation->FrameLength()));
	}
	
	void AnimationPlayer::updateVec3TrackView(TrackView<glm::vec3>& tv, glm::vec3& val, uint32_t frame)
	{
		if (tv.Key + 1 < tv.Track->Keys.size())
		{
			tv.Track->Update(tv.Key, frame, val);
			if (tv.Track->Keys[tv.Key + 1].Frame <= frame)
				tv.Key++;
		}
	}
}