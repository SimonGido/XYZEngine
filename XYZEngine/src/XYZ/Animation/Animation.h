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
		const Ref<T>& FindTrack() const;
		
		inline float GetCurrentTime() const { return m_CurrentTime; }

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
	inline const Ref<T>& Animation::FindTrack() const
	{
		static_assert(std::is_base_of<Track, T>::value, "Type T must be derived from Track base");
		for (auto& track : m_Tracks)
		{
			if (dynamic_cast<T*>(track.Raw()))
				return track.As<T>();
		}
		return Ref<T>();
	}
}