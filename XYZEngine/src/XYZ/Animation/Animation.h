#pragma once
#include "XYZ/Utils/DataStructures/DynamicPool.h"
#include "XYZ/Asset/Asset.h"
#include "XYZ/Scene/SceneEntity.h"

#include <glm/glm.hpp>

namespace XYZ {


	template <typename T>
	static T Interpolate(const T& start, const T& end, float startTime, float endTime, float time);

	template <typename T>
	static void Interpolate(T& out, const T& start, const T& end, float startTime, float endTime, float time);

	template <typename T>
	struct KeyFrame
	{
		bool operator <(const KeyFrame<T>& other) const
		{
			return EndTime < other.EndTime;
		}
		bool operator >(const KeyFrame<T>& other) const
		{
			return EndTime > other.EndTime;
		}
	
		T Value;
		float EndTime = 0.0f;
	};


	template <typename T>
	class Property
	{
	public:
		Property() = default;

		bool  Update(T& val, float time);
		void  Reset() { m_CurrentFrame = 0; }
		void  AddKeyFrame(const KeyFrame<T>& key) { m_Keys.push_back(key); }
		float Length() const;
		

	private:
		std::vector<KeyFrame<T>> m_Keys;
		size_t m_CurrentFrame = 0;
	};

	template<typename T>
	inline bool Property<T>::Update(T& val, float time)
	{
		size_t& current = m_CurrentFrame;
		if (current + 1 < m_Keys.size())
		{
			const KeyFrame<T>& key = m_Keys[current];
			const KeyFrame<T>& next = m_Keys[current + 1];
			Interpolate<T>(val, key.Value, next.Value, key.EndTime, next.EndTime, time);
			if (time >= next.EndTime)
				current++;
			return false;
		}
		return true;
	}

	template<typename T>
	inline float Property<T>::Length() const
	{
		if (m_Keys.empty())
			return 0.0f;
		return m_Keys.back().EndTime;
	}

	class ITrack : public RefCount
	{
	public:
		ITrack(SceneEntity entity);
		ITrack(const ITrack& other);

		virtual ~ITrack() = default;
		virtual bool  Update(float time) = 0;
		virtual void  Reset() = 0;
		virtual float Length() = 0;

	protected:
		SceneEntity m_Entity;
	};

	template <typename T>
	class Track : public ITrack
	{
	public:
		Track(SceneEntity entity);

		virtual bool  Update(float time) override;
		virtual void  Reset() override;
		virtual float Length() override;

	private:
		Property<T> m_Property;
	};

	template<typename T>
	inline Track<T>::Track(SceneEntity entity)
		:
		ITrack(entity)
	{
	}

	template<typename T>
	inline bool Track<T>::Update(float time)
	{
		return m_Property.Update(m_Entity.GetComponent<T>());
	}

	template<typename T>
	inline void Track<T>::Reset()
	{
		m_Property.Reset();
	}

	template<typename T>
	inline float Track<T>::Length()
	{
		return m_Property.Length();
	}

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
		std::vector<Ref<ITrack>> m_Tracks;

		float m_Length;
		float m_CurrentTime;
		bool  m_Repeat;
	};
	
	
	template<typename T>
	inline void Animation::CreateTrack()
	{
		static_assert(std::is_base_of<ITrack, T>::value, "Type T must be derived from Track base");
		m_Tracks.push_back(Ref<T>::Create(m_Entity));
	}

	template<typename T>
	inline const Ref<T>& Animation::FindTrack() const
	{
		static_assert(std::is_base_of<ITrack, T>::value, "Type T must be derived from Track base");
		for (auto& track : m_Tracks)
		{
			if (dynamic_cast<T*>(track.Raw()))
				return track.As<T>();
		}
		return Ref<T>();
	}
}