#pragma once
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Utils/Delegate.h"


#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>

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

	class IProperty
	{
	public:
		virtual ~IProperty() = default;

		virtual bool Update(uint32_t frame) = 0;
		virtual void SetCurrentKey(uint32_t frame) = 0;
		virtual void SetSceneEntity(const SceneEntity& entity) = 0;
		virtual void Reset() = 0;
		virtual void RemoveKeyFrame(uint32_t frame) = 0;
		virtual void SetKeyFrame(uint32_t frame, size_t index) = 0;
		virtual void SetFrames(uint32_t* frames, size_t count) = 0;

		virtual const SceneEntity& GetSceneEntity()	  const = 0;
		virtual const std::string& GetPath()		  const = 0;
		virtual const std::string& GetValueName()	  const = 0;
		virtual const std::string& GetComponentName() const = 0;
	};

	template <typename T>
	class Property : public IProperty
	{
	public:	
		Property(const std::string& path);
		virtual ~Property() override;

		Property(const Property<T>& other);
		Property(Property<T>&& other) noexcept;
		Property<T>& operator=(const Property<T>& other);
		Property<T>& operator=(Property<T>&& other) noexcept;

		virtual bool Update(uint32_t frame) override;
		virtual void SetCurrentKey(uint32_t frame) override;
		virtual void SetSceneEntity(const SceneEntity& entity) override;
		virtual void Reset() override { m_CurrentKey = 0; }
		virtual void RemoveKeyFrame(uint32_t frame) override;
		virtual void SetKeyFrame(uint32_t frame, size_t index) override;
		virtual void SetFrames(uint32_t* frames, size_t count) override;

		virtual const SceneEntity& GetSceneEntity()		  const override { return m_Entity; }
		virtual const std::string& GetPath()			  const override { return m_Path; }
		virtual const std::string& GetValueName()		  const override { return m_ValueName; }
		virtual const std::string& GetComponentName()	  const override { return m_ComponentName; }
		


		template <typename ComponentType, uint16_t valIndex>
		void Init();

		bool AddKeyFrame(const KeyFrame<T>& key);
		void SetKeyValue(const T& value, size_t index);
		T    GetValue(uint32_t frame) const;
	

		bool							Empty()						  const;
		bool							HasKeyAtFrame(uint32_t frame) const;

		uint32_t						Length()					  const;
		int64_t							FindKey(uint32_t frame)		  const;
		std::vector<KeyFrame<T>>&	    GetKeyFrames()				  { return m_Keys; }	
	private:
		bool isKeyInRange() const { return m_CurrentKey + 1 < m_Keys.size(); }

		template <typename ComponentType, uint16_t valIndex>
		static T* getReference(SceneEntity& entity);
	
	private:
		T*								   m_Value;
		SceneEntity						   m_Entity;
		
		std::string						   m_Path;
		std::string						   m_ValueName;
		std::string						   m_ComponentName;
	
		std::vector<KeyFrame<T>>		   m_Keys;


		Delegate<T*(SceneEntity& entity)>  m_GetPropertyReference;	
		uint16_t						   m_ValueIndex  = UINT16_MAX;
		uint16_t						   m_ComponentID = UINT16_MAX;
		size_t							   m_CurrentKey  = MAXSIZE_T;
	};

	template<typename T>
	inline Property<T>::Property(const std::string& path)
		:
		m_Value(nullptr),
		m_Path(path)
	{
	}

	template<typename T>
	inline Property<T>::~Property()
	{
	}
	template<typename T>
	inline Property<T>::Property(const Property<T>& other)
		:
		m_Value(other.m_Value),
		m_Entity(other.m_Entity),
		m_Path(other.m_Path),
		m_ValueName(other.m_ValueName),
		m_ComponentName(other.m_ComponentName),
		m_ValueIndex(other.m_ValueIndex),
		m_ComponentID(other.m_ComponentID),
		m_GetPropertyReference(other.m_GetPropertyReference),
		m_Keys(other.m_Keys),
		m_CurrentKey(other.m_CurrentKey)
	{
	}
	template<typename T>
	inline Property<T>::Property(Property<T>&& other) noexcept
		:
		m_Value(other.m_Value),
		m_Entity(std::move(other.m_Entity)),
		m_Path(std::move(other.m_Path)),
		m_ValueName(std::move(other.m_ValueName)),
		m_ComponentName(std::move(other.m_ComponentName)),
		m_ValueIndex(other.m_ValueIndex),
		m_ComponentID(other.m_ComponentID),
		m_GetPropertyReference(std::move(other.m_GetPropertyReference)),
		m_Keys(std::move(other.m_Keys)),
		m_CurrentKey(other.m_CurrentKey)
	{
	}
	template<typename T>
	inline Property<T>& Property<T>::operator=(const Property<T>& other)
	{
		m_Value				  = other.m_Value;
		m_Entity			  = other.m_Entity;
		m_Path				  = other.m_Path;
		m_ValueName			  = other.m_ValueName;
		m_ComponentName		  = other.m_ComponentName;
		m_ValueIndex		  = other.m_ValueIndex;
		m_ComponentID		  = other.m_ComponentID;
		m_GetPropertyReference = other.m_GetPropertyReference;
		m_Keys				  = other.m_Keys;
		m_CurrentKey		  = other.m_CurrentKey;
		return *this;
	}
	template<typename T>
	inline Property<T>& Property<T>::operator=(Property<T>&& other) noexcept
	{
		m_Value				   = other.m_Value;
		m_Entity			   = std::move(other.m_Entity);
		m_Path				   = std::move(other.m_Path);
		m_ValueName			   = std::move(other.m_ValueName);
		m_ComponentName		   = std::move(other.m_ComponentName);
		m_ValueIndex		   = other.m_ValueIndex;
		m_ComponentID		   = other.m_ComponentID;
		m_GetPropertyReference = std::move(other.m_GetPropertyReference);
		m_Keys				   = std::move(other.m_Keys);
		m_CurrentKey		   = other.m_CurrentKey;
		return *this;
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
	inline void Property<T>::SetKeyFrame(uint32_t frame, size_t index)
	{
		m_Keys[index].Frame = frame;
		std::sort(m_Keys.begin(), m_Keys.end());
	}
	template<typename T>
	inline void Property<T>::SetFrames(uint32_t* frames, size_t count)
	{
		XYZ_ASSERT(count == m_Keys.size(), "");
		for (size_t i = 0; i < count; ++i)
			m_Keys[i].Frame = frames[i];
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
	inline int64_t Property<T>::FindKey(uint32_t frame) const
	{
		if (!m_Keys.empty())
		{
			for (size_t i = 0; i < m_Keys.size() - 1; ++i)
			{
				const auto& key = m_Keys[i];
				const auto& next = m_Keys[i + 1];
				if (frame >= key.Frame && frame < next.Frame)
					return i;
			}
			return m_Keys.size() - 1;
		}
		return -1;
	}

	template<typename T>
	inline T Property<T>::GetValue(uint32_t frame) const
	{
		const int64_t current = FindKey(frame);
		if (current == -1)
			return T();

		const int64_t next = current + 1;
		if (next == m_Keys.size())
			return m_Keys[current].Value;

		const auto& currKey = m_Keys[current];
		const auto& nextKey = m_Keys[next];

		if constexpr (
			   std::is_same_v<T, glm::mat4>
			|| std::is_same_v<T, glm::vec4>
			|| std::is_same_v<T, glm::vec3>
			|| std::is_same_v<T, glm::vec2>
			|| std::is_same_v<T, float>)
		{
			const uint32_t length = nextKey.Frame - currKey.Frame;
			const uint32_t passed = frame - currKey.Frame;
			return glm::lerp(currKey.Value, nextKey.Value, (float)passed / (float)length);
		}
		else if constexpr (std::is_integral_v<T>)
		{
			const uint32_t length = nextKey.Frame - currKey.Frame;
			const uint32_t passed = frame - currKey.Frame;
			return currKey.Value + (nextKey.Value - currKey.Value) * (float)passed / (float)length;
		}
		else
		{
			return currKey.Value;
		}
	}
	
	template<typename T>
	template<typename ComponentType, uint16_t valIndex>
	inline void Property<T>::Init()
	{
		m_GetPropertyReference.Connect<Property<T>::getReference<ComponentType, valIndex>>();
		m_ValueIndex = valIndex;
		m_ComponentID = Component<ComponentType>::ID();
		m_ComponentName = Reflection<ComponentType>::sc_ClassName;
		m_ValueName = std::string(Reflection<ComponentType>::sc_VariableNames[valIndex]);
	}

	template<typename T>
	template<typename ComponentType, uint16_t valIndex>
	inline T* Property<T>::getReference(SceneEntity& entity)
	{
		if (entity.IsValid() && entity.HasComponent<ComponentType>())
			return &Reflection<ComponentType>::Get<valIndex>(entity.GetComponent<ComponentType>());
		return nullptr;
	}

}