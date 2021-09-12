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

		virtual bool  Update(uint32_t frame) = 0;
		virtual void  Reset() = 0;
		virtual uint32_t Length() const = 0;

		SceneEntity GetSceneEntity() const { return m_Entity; }
		TrackType	GetType() const { return m_Type; }
	private:
		virtual void  updatePropertyCurrentKey(uint32_t frame) = 0;
	
	protected:
		SceneEntity m_Entity;
		TrackType   m_Type = TrackType::None;

		friend class Animation;
	};

	template <typename T>
	struct KeyFrame
	{
		bool operator <(const KeyFrame<T>& other) const
		{
			return EndFrame < other.EndFrame;
		}
		bool operator >(const KeyFrame<T>& other) const
		{
			return EndFrame > other.EndFrame;
		}

		T		 Value;
		uint32_t EndFrame = 0;
	};


	template <typename T>
	class Property
	{
	public:
		Property() = default;

		bool     Update(T& val, uint32_t frame);
		void     UpdateCurrentKey(uint32_t frame);
		void     Reset() { m_CurrentKey = 0; }
		void     AddKeyFrame(const KeyFrame<T>& key);
		void     RemoveKeyFrame(uint32_t frame);
		uint32_t Length() const;

	private:
		bool isKeyInRange() const { return m_CurrentKey + 1 < m_Keys.size(); }
	
	private:
		std::vector<KeyFrame<T>> m_Keys;
		size_t					 m_CurrentKey = 0;
	};

	
	template<typename T>
	inline void Property<T>::UpdateCurrentKey(uint32_t frame)
	{
		if (m_Keys.empty())
		{
			m_CurrentKey = 0;
			return;
		}
	
		for (size_t i = 0; i < m_Keys.size() - 1; ++i)
		{
			const auto& key = m_Keys[i];
			const auto& nextKey = m_Keys[i + 1];
			if (frame > key.EndFrame && frame <= nextKey.EndFrame)
			{
				m_CurrentKey = i;
				return;
			}
		}
	}

	template<typename T>
	inline void Property<T>::AddKeyFrame(const KeyFrame<T>& key)
	{
		m_Keys.push_back(key);
		std::sort(m_Keys.begin(), m_Keys.end());
	}

	template<typename T>
	inline void Property<T>::RemoveKeyFrame(uint32_t frame)
	{
		for (auto it = m_Keys.begin(); it != m_Keys.end();)
		{
			if (it->EndFrame == frame)
			{
				it = m_Keys.erase(it);
				return;
			}
			else
			{
				it++;
			}
		}
	}

	template<typename T>
	inline uint32_t Property<T>::Length() const
	{
		if (m_Keys.empty())
			return 0;
		return m_Keys.back().EndFrame;
	}
}