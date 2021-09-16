#pragma once
#include "XYZ/Core/Ref.h"
#include "XYZ/Scene/SceneEntity.h"

#include "Animatable.h"

namespace XYZ {


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



	using SetPropertyRefFn = std::function<void(Animatable& animatable, void* ref)>;
	
	template <typename T>
	class Property
	{
	public:	
		Property(Animatable* animatable, const SetPropertyRefFn& callback, const SceneEntity& entity);

		bool		 Update(uint32_t frame);
		void		 SetReference();
		void		 SetCurrentKey(uint32_t frame);
		void		 Reset() { m_CurrentKey = 0; }

		void		 AddKeyFrame(const KeyFrame<T>& key);
		void		 RemoveKeyFrame(uint32_t frame);
		uint32_t	 Length() const;
	private:
		bool isKeyInRange() const { return m_CurrentKey + 1 < m_Keys.size(); }
	
	private:
		T*						 m_Value;
		Animatable*				 m_Animatable;
		SceneEntity				 m_Entity;
		SetPropertyRefFn		 m_SetPropertyCallback;
		std::vector<KeyFrame<T>> m_Keys;
		size_t					 m_CurrentKey = 0;
	};

	
	template<typename T>
	inline Property<T>::Property(Animatable* animatable, const SetPropertyRefFn& callback, const SceneEntity& entity)
		:
		m_Animatable(animatable),
		m_Entity(entity),
		m_SetPropertyCallback(callback)
	{
	}
	template<typename T>
	inline void Property<T>::SetReference()
	{
		m_SetPropertyCallback(*m_Animatable, *m_Value);
	}
	template<typename T>
	inline void Property<T>::SetCurrentKey(uint32_t frame)
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