#pragma once
#include "XYZ/Asset/Asset.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"
#include "AnimationPlayer.h"
#include "Property.h"

#include <glm/glm.hpp>

namespace XYZ {


	class Animation : public Asset
	{
	public:
		Animation();
		virtual ~Animation() override;

		template <typename ComponentType, typename ValueType, uint16_t valIndex>
		void AddProperty(std::string_view path);

		template <typename ValueType>
		void RemoveProperty(std::string_view path, std::string_view componentName, std::string_view valueName);

		template <typename ValueType>
		Property<ValueType>* GetProperty(std::string_view path, std::string_view componentName, std::string_view valueName);


		void SetFrequency(uint32_t framesPerSecond);
		void SetNumFrames(uint32_t numFrames) { m_NumFrames = numFrames; }
		void SetRepeat(bool repeat)			  { m_Repeat = repeat; }

		bool HasProperty(std::string_view componentName, std::string_view varName, std::string_view path) const;
		bool Empty() const;

		inline uint32_t	GetNumFrames()    const { return m_NumFrames; }
		inline uint32_t GetFrequency()    const { return m_Frequency; }

		inline float	GetFrameLength()  const { return m_FrameLength; }
		inline bool		GetRepeat()	      const { return m_Repeat; }
		size_t			GetPropertyCount() const;

		template <typename T>
		constexpr std::vector<Property<T>>& GetProperties();
	
		template <typename T>
		constexpr const std::vector<Property<T>>& GetProperties() const;

		static AssetType GetStaticType() { return AssetType::Animation; }

	private:
		template <typename T>
		constexpr std::vector<Property<T>>* getProperties();

		template <typename T>
		static void	removeFromContainer(std::vector<T>& container, std::string_view path, std::string_view valueName, std::string_view componentName);

		template <typename T>
		static T*   findInContainer(std::vector<T>& container, std::string_view path, std::string_view valueName, std::string_view componentName);

		template <typename T>
		static bool propertyContainerHasVariable(const std::vector<Property<T>>& container, std::string_view className, std::string_view varName, std::string_view path);
			
		void		clearProperties();
	private:
		// TODO: it might be possible to store them inside variant, keep them sorted by type and store count of each type
		std::vector<Property<glm::vec4>> m_Vec4Properties;
		std::vector<Property<glm::vec3>> m_Vec3Properties;
		std::vector<Property<glm::vec2>> m_Vec2Properties;
		std::vector<Property<float>>     m_FloatProperties;
		std::vector<Property<void*>>	 m_PointerProperties;
		
		uint32_t m_NumFrames;
		uint32_t m_Frequency;

		float    m_FrameLength;
		bool     m_Repeat;


		friend class AnimationPlayer;
	};
	

	template<typename ComponentType, typename ValueType, uint16_t valIndex>
	inline void Animation::AddProperty(std::string_view path)
	{
		Property<ValueType> prop = Property<ValueType>(std::string(path));
		prop.Setup<ComponentType, valIndex>();

		auto props = getProperties<ValueType>();
		if (props)
			props->push_back(prop);
	}

	template<typename ValueType>
	inline void Animation::RemoveProperty(std::string_view path, std::string_view componentName, std::string_view valueName)
	{
		auto props = getProperties<ValueType>();
		if (props)
			removeFromContainer(*props, path, valueName, componentName);
	}

	template <typename ValueType>
	inline Property<ValueType>* Animation::GetProperty(std::string_view path, std::string_view componentName, std::string_view valueName)
	{
		Property<ValueType>* result = nullptr;
		auto props = getProperties<ValueType>();
		if (props)
			result = findInContainer(*props, path, valueName, componentName);

		return result;
	}


	template<typename T>
	inline constexpr std::vector<Property<T>>& Animation::GetProperties()
	{
		if constexpr (std::is_same_v<T, glm::vec4>)
			return m_Vec4Properties;
		else if constexpr (std::is_same_v<T, glm::vec3>)
			return m_Vec3Properties;
		else if constexpr (std::is_same_v<T, glm::vec2>)
			return m_Vec2Properties;
		else if constexpr (std::is_same_v<T, float>)
			return m_FloatProperties;
		else if constexpr (std::is_same_v<T, void*>)
			return m_PointerProperties;
	}

	template<typename T>
	inline constexpr const std::vector<Property<T>>& Animation::GetProperties() const
	{
		if constexpr (std::is_same_v<T, glm::vec4>)
			return m_Vec4Properties;
		else if constexpr (std::is_same_v<T, glm::vec3>)
			return m_Vec3Properties;
		else if constexpr (std::is_same_v<T, glm::vec2>)
			return m_Vec2Properties;
		else if constexpr (std::is_same_v<T, float>)
			return m_FloatProperties;
		else if constexpr (std::is_same_v<T, void*>)
			return m_PointerProperties;
	}

	template<typename T>
	inline constexpr std::vector<Property<T>>* Animation::getProperties()
	{
		if constexpr (std::is_same_v<T, glm::vec4>)
			return &m_Vec4Properties;
		else if constexpr (std::is_same_v<T, glm::vec3>)
			return &m_Vec3Properties;
		else if constexpr (std::is_same_v<T, glm::vec2>)
			return &m_Vec2Properties;
		else if constexpr (std::is_same_v<T, float>)
			return &m_FloatProperties;
		else if constexpr (std::is_same_v<T, void*>)
			return &m_PointerProperties;
		else
			return nullptr;
	}

	template<typename T>
	void Animation::removeFromContainer(std::vector<T>& container, std::string_view path, std::string_view valueName, std::string_view componentName)
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
	T* Animation::findInContainer(std::vector<T>& container, std::string_view path, std::string_view valueName, std::string_view componentName)
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
	bool Animation::propertyContainerHasVariable(const std::vector<Property<T>>& container, std::string_view className, std::string_view varName, std::string_view path)
	{
		for (const auto& it : container)
		{
			if (it.GetComponentName() == className && it.GetValueName() == varName && it.GetPath() == path)
				return true;
		}
		return false;
	}
}