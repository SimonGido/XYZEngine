#pragma once
#include "XYZ/Asset/Asset.h"
#include "XYZ/Scene/SceneEntity.h"
#include "Property.h"

#include <glm/glm.hpp>

namespace XYZ {

	class Animation : public Asset
	{
	public:
		Animation();
		virtual ~Animation() override;

		template <typename ComponentType, typename ValueType>
		void AddProperty(const SceneEntity& entity, const std::string& valueName);

		template <typename ComponentType, typename ValueType>
		void RemoveProperty(const SceneEntity& entity, const std::string& valueName);

		template <typename ComponentType, typename ValueType>
		Property<ValueType>* GetProperty(const SceneEntity& entity, const std::string& valueName);

		void Update(Timestep ts);
		void UpdateLength();
		void SetFrequency(uint32_t framesPerSecond);
		void SetCurrentFrame(uint32_t frame);
		void SetNumFrames(uint32_t numFrames) { m_NumFrames = numFrames; }
		void SetRepeat(bool repeat)			  { m_Repeat = repeat; }

		bool PropertyHasVariable(const char* componentName, const char* varName, const SceneEntity& entity) const;

		inline uint32_t	GetNumFrames()    const { return m_NumFrames; }
		inline uint32_t	GetCurrentFrame() const { return m_CurrentFrame; }
		inline uint32_t GetFrequency()    const { return m_Frequency; }
		inline float	GetFrameLength()  const { return m_FrameLength; }
		inline bool		GetRepeat()	      const { return m_Repeat; }
	
	private:
		void updateProperties(uint32_t frame);
		void setPropertiesKey(uint32_t frame);
		void resetProperties();

		template <typename ValueType>
		void	    addPropertySpecialized(const Property<ValueType>& prop);
		
		template <typename ValueType>
		void	    removePropertySpecialized(const SceneEntity& entity, const std::string& valueName, const std::string& componentName);
		
		template <typename T>
		Property<T>* getPropertySpecialized(const SceneEntity& entity, const std::string& valueName, const std::string& componentName);
		
		template <typename T>
		static void	removeFromContainer(std::vector<T>& container, const SceneEntity& entity, const std::string& valueName, const std::string& componentName);

		template <typename T>
		static T*   findInContainer(std::vector<T>& container, const SceneEntity& entity, const std::string& valueName, const std::string& componentName);

		template <typename T>
		static bool propertyHasVariable(const std::vector<Property<T>>& container, const char* className, const char* varName, const SceneEntity& entity);
			
		void		clearProperties();
	private:
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
	};
	

	template<typename ComponentType, typename ValueType>
	inline void Animation::AddProperty(const SceneEntity& entity, const std::string& valueName)
	{
		SetPropertyRefFn<ValueType> callback = [](SceneEntity ent, ValueType** ref, const std::string& varName) {
			*ref = &Reflection<ComponentType>::GetByName<ValueType>(varName.c_str(), ent.GetComponent<ComponentType>());
		};
		addPropertySpecialized<ValueType>(Property<ValueType>(callback, entity, valueName, Reflection<ComponentType>::sc_ClassName, Component<ComponentType>::ID()));		
	}

	template<typename ComponentType, typename ValueType>
	inline void Animation::RemoveProperty(const SceneEntity& entity, const std::string& valueName)
	{
		removePropertySpecialized<ValueType>(entity, valueName, Reflection<ComponentType>::sc_ClassName);
	}

	template <typename ComponentType, typename ValueType>
	inline Property<ValueType>* Animation::GetProperty(const SceneEntity& entity, const std::string& valueName)
	{
		return getPropertySpecialized<ValueType>(entity, valueName, Reflection<ComponentType>::sc_ClassName);
	}

	template<typename T>
	void Animation::removeFromContainer(std::vector<T>& container, const SceneEntity& entity, const std::string& valueName, const std::string& componentName)
	{
		for (size_t i = 0; i < container.size(); ++i)
		{
			auto& prop = container[i];
			if (prop.GetSceneEntity() == entity && prop.GetValueName() == valueName && prop.GetComponentName() == componentName)
			{
				container.erase(container.begin() + i);
				return;
			}
		}
	}
	template<typename T>
	T* Animation::findInContainer(std::vector<T>& container, const SceneEntity& entity, const std::string& valueName, const std::string& componentName)
	{
		for (size_t i = 0; i < container.size(); ++i)
		{
			auto& prop = container[i];
			if (prop.GetSceneEntity() == entity && prop.GetValueName() == valueName && prop.GetComponentName() == componentName)
			{
				return &prop;
			}
		}
		XYZ_ASSERT(false, "Not found in container");
		return nullptr;
	}
	template <typename T>
	bool Animation::propertyHasVariable(const std::vector<Property<T>>& container, const char* className, const char* varName, const SceneEntity& entity)
	{
		for (const auto& it : container)
		{
			if (it.GetComponentName() == className && it.GetValueName() == varName && it.GetSceneEntity() == entity)
				return true;
		}
		return false;
	}
}