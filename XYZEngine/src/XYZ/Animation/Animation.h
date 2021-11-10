#pragma once
#include "XYZ/Asset/Asset.h"
#include "XYZ/Scene/SceneEntity.h"
#include "Property.h"
#include "XYZ/Scene/Components.h"

#include <glm/glm.hpp>

namespace XYZ {

	class Animation : public Asset
	{
	public:
		Animation();
		virtual ~Animation() override;

		template <typename ComponentType, typename ValueType>
		void AddProperty(const std::string& path, const std::string& valueName, uint16_t valIndex);

		template <typename ComponentType, typename ValueType>
		void RemoveProperty(const std::string& path, const std::string& valueName);

		template <typename ComponentType, typename ValueType>
		Property<ValueType>* GetProperty(const std::string& path, const std::string& valueName);

		void Update(Timestep ts);
		void UpdateLength();
		
		void SetFrequency(uint32_t framesPerSecond);
		void SetCurrentFrame(uint32_t frame);
		void SetNumFrames(uint32_t numFrames) { m_NumFrames = numFrames; }
		void SetRepeat(bool repeat)			  { m_Repeat = repeat; }

		bool PropertyHasVariable(const char* componentName, const char* varName, const std::string& path) const;

		inline uint32_t	GetNumFrames()    const { return m_NumFrames; }
		inline uint32_t	GetCurrentFrame() const { return m_CurrentFrame; }
		inline uint32_t GetFrequency()    const { return m_Frequency; }
		inline float	GetFrameLength()  const { return m_FrameLength; }
		inline bool		GetRepeat()	      const { return m_Repeat; }

		const std::vector<Property<glm::vec4>>& GetVec4Properties()    const { return m_Vec4Properties;};
		const std::vector<Property<glm::vec3>>& GetVec3Properties()    const { return m_Vec3Properties;};
		const std::vector<Property<glm::vec2>>& GetVec2Properties()    const { return m_Vec2Properties;};
		const std::vector<Property<float>>    & GetFloatProperties()   const { return m_FloatProperties;};
		const std::vector<Property<void*>>	  & GetPointerProperties() const { return m_PointerProperties;};

		static AssetType GetStaticType() { return AssetType::Animation; }

	private:
		void setSceneEntity(const SceneEntity& entity);
		void updateProperties(uint32_t frame);
		void setPropertiesKey(uint32_t frame);
		void resetProperties();

		template <typename ValueType>
		void	    addPropertySpecialized(const Property<ValueType>& prop);
		
		template <typename ValueType>
		void	    removePropertySpecialized(const std::string& path, const std::string& valueName, const std::string& componentName);
		
		template <typename T>
		void		setPropertySceneEntity(std::vector<Property<T>>& container);

		template <typename T>
		Property<T>* getPropertySpecialized(const std::string& path, const std::string& valueName, const std::string& componentName);
		
		template <typename T>
		static void	removeFromContainer(std::vector<T>& container, const std::string& path, const std::string& valueName, const std::string& componentName);

		template <typename T>
		static T*   findInContainer(std::vector<T>& container, const std::string& path, const std::string& valueName, const std::string& componentName);

		template <typename T>
		static bool propertyHasVariable(const std::vector<Property<T>>& container, const char* className, const char* varName, const std::string& path);
			
		void		clearProperties();
	private:
		SceneEntity m_Entity;


		std::vector<Property<glm::vec4>> m_Vec4Properties;
		std::vector<Property<glm::vec3>> m_Vec3Properties;
		std::vector<Property<glm::vec2>> m_Vec2Properties;
		std::vector<Property<float>>     m_FloatProperties;
		std::vector<Property<void*>>	 m_PointerProperties;
		
		uint32_t m_NumFrames;
		uint32_t m_CurrentFrame;
		uint32_t m_Frequency;
		float	 m_CurrentTime;
		float    m_FrameLength;
		bool     m_Repeat;

		friend class Animator;
	};
	

	template<typename ComponentType, typename ValueType>
	inline void Animation::AddProperty(const std::string& path, const std::string& valueName, uint16_t valIndex)
	{
		SetPropertyRefFn<ValueType> callback = [](SceneEntity ent, ValueType** ref, const uint16_t varIndex) {
			if (ent.IsValid() && ent.HasComponent<ComponentType>())
				*ref = &Reflection<ComponentType>::GetByIndex<ValueType>(varIndex, ent.GetComponent<ComponentType>());
			else
				*ref = nullptr;
		};
		addPropertySpecialized<ValueType>(Property<ValueType>(callback, path, valueName, Reflection<ComponentType>::sc_ClassName, valIndex, Component<ComponentType>::ID()));		
	}

	template<typename ComponentType, typename ValueType>
	inline void Animation::RemoveProperty(const std::string& path, const std::string& valueName)
	{
		removePropertySpecialized<ValueType>(path, valueName, Reflection<ComponentType>::sc_ClassName);
	}

	template <typename ComponentType, typename ValueType>
	inline Property<ValueType>* Animation::GetProperty(const std::string& path, const std::string& valueName)
	{
		return getPropertySpecialized<ValueType>(path, valueName, Reflection<ComponentType>::sc_ClassName);
	}

	template<typename T>
	inline void Animation::setPropertySceneEntity(std::vector<Property<T>>& container)
	{
		auto& relationship = m_Entity.GetComponent<Relationship>();
		for (auto& prop : container)
		{
			auto ecs		 = m_Entity.GetECS();
			const auto& path = prop.GetPath();
			if (!path.empty())
			{
				Entity result = relationship.FindByName(*ecs, path);
				prop.SetSceneEntity({ result, m_Entity.GetScene() });
			}
			prop.SetReference();
		}
	}

	template<typename T>
	void Animation::removeFromContainer(std::vector<T>& container, const std::string& path, const std::string& valueName, const std::string& componentName)
	{
		for (size_t i = 0; i < container.size(); ++i)
		{
			auto& prop = container[i];
			if (prop.GetPath() == path && prop.GetValueName() == valueName && prop.GetComponentName() == componentName)
			{
				container.erase(container.begin() + i);
				return;
			}
		}
	}
	template<typename T>
	T* Animation::findInContainer(std::vector<T>& container, const std::string& path, const std::string& valueName, const std::string& componentName)
	{
		for (size_t i = 0; i < container.size(); ++i)
		{
			auto& prop = container[i];
			if (prop.GetPath() == path && prop.GetValueName() == valueName && prop.GetComponentName() == componentName)
			{
				return &prop;
			}
		}
		return nullptr;
	}
	template <typename T>
	bool Animation::propertyHasVariable(const std::vector<Property<T>>& container, const char* className, const char* varName, const std::string& path)
	{
		for (const auto& it : container)
		{
			if (it.GetComponentName() == className && it.GetValueName() == varName && it.GetPath() == path)
				return true;
		}
		return false;
	}
}