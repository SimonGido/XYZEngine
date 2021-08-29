#pragma once
#include "XYZ/Core/Ref.h"
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {

	enum class TrackType
	{
		Transform, SpriteRenderer, None
	};

	class Track : public RefCount
	{
	public:
		Track(SceneEntity entity);
		virtual ~Track() = default;

		virtual bool  Update(float time) = 0;
		virtual void  Reset() = 0;
		virtual float Length() const = 0;

		SceneEntity GetSceneEntity() const { return m_Entity; }
		TrackType	GetType() const { return m_Type; }
	protected:
		SceneEntity m_Entity;
		TrackType   m_Type = TrackType::None;
	};

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
	inline float Property<T>::Length() const
	{
		if (m_Keys.empty())
			return 0.0f;
		return m_Keys.back().EndTime;
	}
}