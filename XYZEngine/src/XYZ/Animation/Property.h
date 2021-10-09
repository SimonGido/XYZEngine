#pragma once
#include "XYZ/Core/Ref.h"
#include "XYZ/Scene/SceneEntity.h"


namespace XYZ {


	template <typename T>
	struct KeyFrame
	{
		bool operator <(const KeyFrame<T>& other) const
		{
			return Frame < other.Frame;
		}
		bool operator >(const KeyFrame<T>& other) const
		{
			return Frame > other.Frame;
		}

		T		 Value;
		uint32_t Frame = 0;
	};


	template <typename T>
	using SetPropertyRefFn = std::function<void(SceneEntity entity, T** ref, const std::string& varName)>;
	
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


		bool Update(uint32_t frame);
		void SetReference();
		void SetCurrentKey(uint32_t frame);
		void SetSceneEntity(const SceneEntity& entity);
		void Reset() { m_CurrentKey = 0; }
			 
		bool AddKeyFrame(const KeyFrame<T>& key);
		void RemoveKeyFrame(uint32_t frame);
		void SetKeyFrame(uint32_t Frame, size_t index);
		void SetKeyValue(const T& value, size_t index);
		T    GetValue(uint32_t frame) const;
		
		bool							Empty()						  const;
		bool							HasKeyAtFrame(uint32_t frame) const;
		uint32_t						Length()					  const;
		size_t							FindKey(uint32_t frame)		  const;
	    const SceneEntity&				GetSceneEntity()   const { return m_Entity; }
		const std::string&				GetValueName()     const { return m_ValueName; }
		const std::string&				GetComponentName() const { return m_ComponentName; }	
		const std::vector<KeyFrame<T>>& GetKeyFrames() const { return m_Keys; }
	private:
		bool   isKeyInRange() const { return m_CurrentKey + 1 < m_Keys.size(); }
		
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
		SetReference();
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
		m_SetPropertyCallback(other.m_SetPropertyCallback),
		m_Keys(other.m_Keys),
		m_CurrentKey(other.m_CurrentKey)
	{
		SetReference();
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
		m_SetPropertyCallback(std::move(other.m_SetPropertyCallback)),
		m_Keys(std::move(other.m_Keys)),
		m_CurrentKey(other.m_CurrentKey)
	{
		SetReference();
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
		m_Keys = other.m_Keys;
		m_CurrentKey = other.m_CurrentKey;
		SetReference();
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
		m_Keys = std::move(other.m_Keys);
		m_CurrentKey = other.m_CurrentKey;
		SetReference();
		return *this;
	}
	template<typename T>
	inline void Property<T>::SetReference()
	{
		m_SetPropertyCallback(m_Entity, &m_Value, m_ValueName);
	}
	template<typename T>
	inline void Property<T>::SetCurrentKey(uint32_t frame)
	{
		if (m_Keys.empty())
		{
			m_CurrentKey = 0;
			return;
		}
		m_CurrentKey = FindKey(frame);
	}

	template<typename T>
	inline void Property<T>::SetSceneEntity(const SceneEntity& entity)
	{
		m_Entity = entity;
		SetReference();
	}

	template<typename T>
	inline bool Property<T>::AddKeyFrame(const KeyFrame<T>& key)
	{
		for (const auto& k : m_Keys)
		{
			if (k.Frame == key.Frame)
				return false;
		}
		m_Keys.push_back(key);
		std::sort(m_Keys.begin(), m_Keys.end());
		return true;
	}

	template<typename T>
	inline void Property<T>::RemoveKeyFrame(uint32_t frame)
	{
		for (auto it = m_Keys.begin(); it != m_Keys.end();)
		{
			if (it->Frame == frame)
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
	inline void Property<T>::SetKeyFrame(uint32_t Frame, size_t index)
	{
		m_Keys[index].Frame = Frame;
		std::sort(m_Keys.begin(), m_Keys.end());
	}

	template<typename T>
	inline void Property<T>::SetKeyValue(const T& value, size_t index)
	{
		m_Keys[index].Value = value;
	}

	template<typename T>
	inline bool Property<T>::Empty() const
	{
		return m_Keys.empty();
	}

	template<typename T>
	inline bool Property<T>::HasKeyAtFrame(uint32_t frame) const
	{
		for (const auto& key : m_Keys)
		{
			if (key.Frame == frame)
				return true;
		}
		return false;
	}

	template<typename T>
	inline uint32_t Property<T>::Length() const
	{
		if (m_Keys.empty())
			return 0;
		return m_Keys.back().Frame;
	}
	template<typename T>
	inline size_t Property<T>::FindKey(uint32_t frame) const
	{
		for (size_t i = 0; i < m_Keys.size() - 1; ++i)
		{
			const auto& key  = m_Keys[i];
			const auto& next = m_Keys[i + 1];
			if (frame >= key.Frame && frame <= next.Frame)
			{
				if (frame == next.Frame)
					return i + 1;
				return i;
			}
		}
		if (!m_Keys.empty())
			return m_Keys.size() - 1;
		return 0;
	}
}