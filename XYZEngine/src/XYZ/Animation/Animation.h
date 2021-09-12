#pragma once
#include "XYZ/Asset/Asset.h"
#include "XYZ/Scene/SceneEntity.h"
#include "Track.h"

#include <glm/glm.hpp>

namespace XYZ {


	class Animation : public Asset
	{
	public:
		Animation();

		void Update(Timestep ts);
		void UpdateLength();
		void SetCurrentFrame(uint32_t frame);
		void SetNumFrames(uint32_t numFrames) { m_NumFrames = numFrames; }
		void SetRepeat(bool repeat) { m_Repeat = repeat; }

		template <typename T>
		void CreateTrack(SceneEntity entity);

		template <typename T>
		void RemoveTrack(SceneEntity entity);
		
		template <typename T>
		Ref<T> FindTrack(SceneEntity entity) const;
		
		inline const std::vector<Ref<Track>>& GetTracks()	   const { return m_Tracks; }
		inline uint32_t						  GetNumFrames()   const { return m_NumFrames; }
		inline uint32_t						  GetTime()        const { return m_CurrentFrame; }
		inline float						  GetFrameLength() const { return m_FrameLength; }
		inline bool							  GetRepeat()	   const { return m_Repeat; }
	private:
		std::vector<Ref<Track>> m_Tracks;

		uint32_t m_NumFrames;
		uint32_t m_CurrentFrame;
		float	 m_CurrentTime;
		float    m_FrameLength;
		bool     m_Repeat;
	};
	
	
	template<typename T>
	inline void Animation::CreateTrack(SceneEntity entity)
	{
		static_assert(std::is_base_of<Track, T>::value, "Type T must be derived from Track base");
		m_Tracks.push_back(Ref<T>::Create(entity));
	}

	template<typename T>
	inline void Animation::RemoveTrack(SceneEntity entity)
	{
		static_assert(std::is_base_of<Track, T>::value, "Type T must be derived from Track base");
		for (auto it = m_Tracks.begin(); it != m_Tracks.end(); ++it)
		{
			if (dynamic_cast<T*>(it->Raw()))
			{
				if ((*it)->GetSceneEntity() == entity)
				{
					m_Tracks.erase(it);
					return;
				}
			}
		}
	}

	template<typename T>
	inline Ref<T> Animation::FindTrack(SceneEntity entity) const
	{
		static_assert(std::is_base_of<Track, T>::value, "Type T must be derived from Track base");
		for (auto& track : m_Tracks)
		{
			if (dynamic_cast<const T*>(track.Raw()))
			{
				if (track->GetSceneEntity() == entity)
				{
					return track.As<T>();
				}
			}
		}
		return Ref<T>();
	}
}