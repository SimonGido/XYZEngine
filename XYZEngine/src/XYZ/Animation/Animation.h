#pragma once
#include "XYZ/Asset/Asset.h"
#include "XYZ/Scene/SceneEntity.h"
#include "Track.h"

#include <glm/glm.hpp>

namespace XYZ {


	class Animation : public Asset
	{
	public:
		Animation(SceneEntity entity);

		void Update(Timestep ts);
		void UpdateLength();
		void SetLength(float length) { m_Length = length; }
		void SetRepeat(bool repeat) { m_Repeat = repeat; }
		void SetCurrentTime(float time) { m_CurrentTime = time; }

		template <typename T>
		void CreateTrack();

		template <typename T>
		void RemoveTrack();
		
		template <typename T>
		Ref<T> FindTrack() const;
		

		inline SceneEntity GetEntity() const { return m_Entity; }
		inline float	   GetLength() const { return m_Length; }
		inline float	   GetCurrentTime() const { return m_CurrentTime; }
		inline bool		   GetRepeat() const { return m_Repeat; }
	private:
		SceneEntity m_Entity;
		std::vector<Ref<Track>> m_Tracks;

		float m_Length;
		float m_CurrentTime;
		bool  m_Repeat;
	};
	
	
	template<typename T>
	inline void Animation::CreateTrack()
	{
		static_assert(std::is_base_of<Track, T>::value, "Type T must be derived from Track base");
		m_Tracks.push_back(Ref<T>::Create(m_Entity));
	}

	template<typename T>
	inline void Animation::RemoveTrack()
	{
		static_assert(std::is_base_of<Track, T>::value, "Type T must be derived from Track base");
		for (auto it = m_Tracks.begin(); it != m_Tracks.end(); ++it)
		{
			if (dynamic_cast<T*>(it->Raw()))
			{
				m_Tracks.erase(it);
				return;
			}
		}
	}

	template<typename T>
	inline Ref<T> Animation::FindTrack() const
	{
		static_assert(std::is_base_of<Track, T>::value, "Type T must be derived from Track base");
		for (auto& track : m_Tracks)
		{
			if (dynamic_cast<const T*>(track.Raw()))
				return track.As<T>();
		}
		return Ref<T>();
	}
}