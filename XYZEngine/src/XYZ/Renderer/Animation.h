#pragma once
#include "XYZ/Utils/DataStructures/DynamicPool.h"
#include "XYZ/Asset/Asset.h"
#include "XYZ/Scene/SceneEntity.h"

#include <glm/glm.hpp>

namespace XYZ {


	template <typename T>
	static T Interpolate(const T& start, const T& end, float startTime, float endTime, float time);


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
		Property(const Property<T>& other);
		Property(Property<T>&& other) noexcept;

		bool  Update(T& val, float time);
		void  Reset() { m_CurrentFrame = 0; }
		void  AddKeyFrame(const KeyFrame<T>& key) { m_Keys.push_back(key); }
		float Length() const
		{
			if (m_Keys.empty())
				return 0.0f;
			return m_Keys.back().EndTime;
		}

	private:
		std::vector<KeyFrame<T>> m_Keys;
		size_t m_CurrentFrame = 0;
	};

	template <typename T>
	Property<T>::Property(const Property<T>& other)
		:
		m_Keys(other.m_Keys),
		m_CurrentFrame(other.m_CurrentFrame)
	{}
	template <typename T>
	Property<T>::Property(Property<T>&& other) noexcept
		:
		m_Keys(std::move(other.m_Keys)),
		m_CurrentFrame(other.m_CurrentFrame)
	{}

	class Track : public DynamicPool::Base
	{
	public:
		Track(SceneEntity entity);
		Track(const Track& other);

		virtual ~Track() = default;
		virtual bool  Update(float time) = 0;
		virtual void  Reset() = 0;
		virtual float Length() = 0;

	protected:
		SceneEntity m_Entity;
	};

	class TransformTrack : public Track
	{
	public:
		enum class PropertyType { Translation, Rotation, Scale };

		TransformTrack(SceneEntity entity);
		TransformTrack(const TransformTrack& other);
		TransformTrack(TransformTrack&& other) noexcept;


		virtual void  OnCopy(uint8_t* buffer) override;
		virtual bool  Update(float time) override;
		virtual void  Reset() override;
		virtual float Length() override;


		template <typename T>
		void AddKeyFrame(const KeyFrame<T>& key, PropertyType type);


	private:
		Property<glm::vec3> m_TranslationProperty;
		Property<glm::vec3> m_RotationProperty;
		Property<glm::vec3> m_ScaleProperty;
	};



	class Animation : public Asset
	{
	public:
		Animation(SceneEntity entity);

		void Update(Timestep ts);
		void UpdateLength();
		void SetLength(float length) { m_Length = length; }
		void SetRepeat(bool repeat) { m_Repeat = repeat; }

		template <typename T>
		void CreateTrack()
		{
			static_assert(std::is_base_of<Track, T>::value, "");
			m_Tracks.Emplace<T>(m_Entity);
		}

		template <typename T>
		T* FindTrack()
		{
			static_assert(std::is_base_of<Track, T>::value, "");
			for (size_t i = 0; i < m_Tracks.Size(); ++i)
			{
				if (auto casted = dynamic_cast<T*>(&m_Tracks[i]))
					return casted;
			}
			return nullptr;
		}

		inline float GetCurrentTime() const { return m_CurrentTime; }

	private:
		SceneEntity m_Entity;

		DynamicPool m_Tracks;

		float m_Length;
		float m_CurrentTime;
		bool  m_Repeat;
	};
	


}