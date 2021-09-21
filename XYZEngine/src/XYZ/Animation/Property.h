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


	template <typename T>
	using SetPropertyRefFn = std::function<void(SceneEntity entity, T* ref, const std::string& varName)>;
	
	template <typename T>
	class Property
	{
	public:	
		Property(const SetPropertyRefFn<T>& callback, const SceneEntity& entity, const std::string& valueName, const std::string& componentName, uint16_t componentID);
		~Property();

		Property(const Property<T>& other);
		Property(Property<T>&& other) noexcept;
		Property<T>& operator=(const Property<T>& other);
		Property<T>& operator=(Property<T>&& other) noexcept;


		bool		       Update(uint32_t frame);
		void		       SetReference();
		void		       SetCurrentKey(uint32_t frame);
		void		       Reset() { m_CurrentKey = 0; }
					       
		void		       AddKeyFrame(const KeyFrame<T>& key);
		void		       RemoveKeyFrame(uint32_t frame);
		
		uint32_t		   Length()			  const;
	    SceneEntity		   GetSceneEntity()   const { return m_Entity; }
		const std::string& GetValueName()     const { return m_ValueName; }
		const std::string& GetComponentName() const { return m_ComponentName; }

	private:
		bool isKeyInRange() const { return m_CurrentKey + 1 < m_Keys.size(); }
	
	private:
		T*						 m_Value;
		SceneEntity				 m_Entity;
		std::string				 m_ValueName;
		std::string				 m_ComponentName;
		uint16_t				 m_ComponentID;
		SetPropertyRefFn<T>		 m_SetPropertyCallback;

		std::vector<KeyFrame<T>> m_Keys;
		size_t					 m_CurrentKey = 0;
	};

	
	template<typename T>
	inline Property<T>::Property(const SetPropertyRefFn<T>& callback, const SceneEntity& entity, const std::string& valueName, const std::string& componentName, uint16_t componentID)
		:
		m_Entity(entity),
		m_ValueName(valueName),
		m_ComponentName(componentName),
		m_ComponentID(componentID),
		m_SetPropertyCallback(callback)
	{
		m_SetPropertyCallback(entity, m_Value, m_ValueName);
		m_Entity.AddOnComponentConstruction(componentID, &Property<T>::SetReference, this);
		m_Entity.AddOnComponentDestruction(componentID, &Property<T>::SetReference, this);
	}
	template<typename T>
	inline Property<T>::~Property()
	{
		m_Entity.RemoveOnComponentConstructionOfInstance(m_ComponentID, this);
		m_Entity.RemoveOnComponentDestructionOfInstance(m_ComponentID, this);
	}
	template<typename T>
	inline Property<T>::Property(const Property<T>& other)
		:
		m_Value(other.m_Value),
		m_Entity(other.m_Entity),
		m_ValueName(other.m_ValueName),
		m_ComponentName(other.m_ComponentName),
		m_ComponentID(other.m_ComponentID),
		m_SetPropertyCallback(other.m_SetPropertyCallback)
	{
		m_Entity.AddOnComponentConstruction(m_ComponentID, &Property<T>::SetReference, this);
		m_Entity.AddOnComponentDestruction(m_ComponentID, &Property<T>::SetReference, this);
	}
	template<typename T>
	inline Property<T>::Property(Property<T>&& other) noexcept
		:
		m_Value(other.m_Value),
		m_Entity(std::move(other.m_Entity)),
		m_ValueName(std::move(other.m_ValueName)),
		m_ComponentName(std::move(other.m_ComponentName)),
		m_ComponentID(other.m_ComponentID),
		m_SetPropertyCallback(std::move(other.m_SetPropertyCallback))
	{
		m_Entity.AddOnComponentConstruction(m_ComponentID, &Property<T>::SetReference, this);
		m_Entity.AddOnComponentDestruction(m_ComponentID, &Property<T>::SetReference, this);
	}
	template<typename T>
	inline Property<T>& Property<T>::operator=(const Property<T>& other)
	{
		m_Value = other.m_Value;
		m_Entity = other.m_Entity;
		m_ValueName = other.m_ValueName;
		m_ComponentName = other.m_ComponentName;
		m_ComponentID = other.m_ComponentID;
		m_SetPropertyCallback = other.m_SetPropertyCallback;
		return *this;
	}
	template<typename T>
	inline Property<T>& Property<T>::operator=(Property<T>&& other) noexcept
	{
		m_Value = other.m_Value;
		m_Entity = std::move(other.m_Entity);
		m_ValueName = std::move(other.m_ValueName);
		m_ComponentName = std::move(other.m_ComponentName);
		m_ComponentID = other.m_ComponentID;
		m_SetPropertyCallback = std::move(other.m_SetPropertyCallback);
		return *this;
	}
	template<typename T>
	inline void Property<T>::SetReference()
	{
		m_SetPropertyCallback(m_Entity, m_Value, m_ValueName);
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