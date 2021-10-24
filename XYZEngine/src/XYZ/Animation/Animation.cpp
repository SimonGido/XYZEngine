#include "stdafx.h"
#include "Animation.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Scene/Components.h"

#include <glm/gtx/matrix_interpolation.hpp>

namespace XYZ {

	Animation::Animation()
		:
		m_NumFrames(0),
		m_CurrentFrame(0),
		m_CurrentTime(0.0f),
		m_Repeat(true)
	{
		SetFrequency(60);
	}

	Animation::~Animation()
	{
	}

	void Animation::Update(Timestep ts)
	{
		if (m_CurrentFrame >= m_NumFrames)
		{
			if (!m_Repeat)
				return;
			resetProperties();
			m_CurrentTime = 0.0f;
			m_CurrentFrame = 0;
		}
		
		updateProperties(m_CurrentFrame);
		m_CurrentTime += ts;
		m_CurrentFrame = static_cast<uint32_t>(std::floor(m_CurrentTime / m_FrameLength));
	}

	void Animation::UpdateLength()
	{

	}

	void Animation::setSceneEntity(const SceneEntity& entity)
	{
		m_Entity = entity;
		setPropertySceneEntity(m_Vec4Properties);
		setPropertySceneEntity(m_Vec3Properties);
		setPropertySceneEntity(m_Vec2Properties);
		setPropertySceneEntity(m_FloatProperties);
		setPropertySceneEntity(m_PointerProperties);
	}

	void Animation::SetFrequency(uint32_t framesPerSecond)
	{
		m_Frequency = framesPerSecond;
		m_FrameLength = 1.0f / m_Frequency;
	}

	void Animation::SetCurrentFrame(uint32_t frame)
	{
		m_CurrentFrame = std::min(m_NumFrames, frame);
		m_CurrentTime = m_CurrentFrame * m_FrameLength;
		setPropertiesKey(m_CurrentFrame);
	}

	bool Animation::PropertyHasVariable(const char* componentName, const char* varName, const std::string& path) const
	{
		if (propertyHasVariable(m_Vec4Properties, componentName, varName, path))
			return true;
		if (propertyHasVariable(m_Vec3Properties, componentName, varName, path))
			return true;
		if (propertyHasVariable(m_Vec2Properties, componentName, varName, path))
			return true;
		if (propertyHasVariable(m_FloatProperties, componentName, varName, path))
			return true;
		if (propertyHasVariable(m_PointerProperties, componentName, varName, path))
			return true;
		return false;
	}

	void Animation::updateProperties(uint32_t frame)
	{
		for (auto& prop : m_Vec4Properties)
			prop.Update(frame);
		for (auto& prop : m_Vec3Properties)
			prop.Update(frame);
		for (auto& prop : m_Vec2Properties)
			prop.Update(frame);
		for (auto& prop : m_FloatProperties)
			prop.Update(frame);
		for (auto& prop : m_PointerProperties)
			prop.Update(frame);
	}

	void Animation::setPropertiesKey(uint32_t frame)
	{
		for (auto& prop : m_Vec4Properties)
		{
			prop.SetCurrentKey(frame);
			prop.Update(frame);
		}
		for (auto& prop : m_Vec3Properties)
		{
			prop.SetCurrentKey(frame);
			prop.Update(frame);
		}
		for (auto& prop : m_Vec2Properties)
		{
			prop.SetCurrentKey(frame);
			prop.Update(frame);
		}
		for (auto& prop : m_FloatProperties)
		{
			prop.SetCurrentKey(frame);
			prop.Update(frame);
		}
		for (auto& prop : m_PointerProperties)
		{
			prop.SetCurrentKey(frame);
			prop.Update(frame);
		}
	}

	void Animation::resetProperties()
	{
		for (auto& prop : m_Vec4Properties)
			prop.Reset();
		for (auto& prop : m_Vec3Properties)
			prop.Reset();
		for (auto& prop : m_Vec2Properties)
			prop.Reset();
		for (auto& prop : m_FloatProperties)
			prop.Reset();
		for (auto& prop : m_PointerProperties)
			prop.Reset();
	}


	void Animation::clearProperties()
	{
		m_Vec4Properties.clear();
		m_Vec3Properties.clear();
		m_Vec2Properties.clear();
		m_FloatProperties.clear();
		m_PointerProperties.clear();
	}

	template<>
	void Animation::addPropertySpecialized(const Property<glm::vec4>& prop)
	{
		m_Vec4Properties.push_back(prop);
	}
	template<>
	void Animation::addPropertySpecialized(const Property<glm::vec3>& prop)
	{
		m_Vec3Properties.push_back(prop);
	}
	template<>
	void Animation::addPropertySpecialized(const Property<glm::vec2>& prop)
	{
		m_Vec2Properties.push_back(prop);
	}
	template<>
	void Animation::addPropertySpecialized(const Property<float>& prop)
	{
		m_FloatProperties.push_back(prop);
	}
	template<>
	void Animation::addPropertySpecialized(const Property<glm::mat4>& prop)
	{
		// TODO: not supported yet
	}
	template<>
	void Animation::addPropertySpecialized(const Property<Ref<Material>>& prop)
	{
		// TODO: not supported yet
	}
	template<>
	void Animation::addPropertySpecialized(const Property<Ref<SubTexture>>& prop)
	{
		// TODO: not supported yet
	}
	template<>
	void Animation::addPropertySpecialized(const Property<uint32_t>& prop)
	{
		// TODO: not supported yet
	}
	template<>
	void Animation::addPropertySpecialized(const Property<bool>& prop)
	{
		// TODO: not supported yet
	}
	template<>
	void Animation::addPropertySpecialized(const Property<void*>& prop)
	{
		// TODO: not supported yet
	}
	template <>
	void Animation::removePropertySpecialized<glm::vec4>(const std::string& path, const std::string& valueName, const std::string& componentName)
	{
		removeFromContainer(m_Vec4Properties, path, valueName, componentName);
	}

	template <>
	void Animation::removePropertySpecialized<glm::vec3>(const std::string& path, const std::string& valueName, const std::string& componentName)
	{
		removeFromContainer(m_Vec3Properties, path, valueName, componentName);
	}
	template <>
	void Animation::removePropertySpecialized<glm::vec2>(const std::string& path, const std::string& valueName, const std::string& componentName)
	{
		removeFromContainer(m_Vec2Properties, path, valueName, componentName);
	}
	template <>
	void Animation::removePropertySpecialized<float>(const std::string& path, const std::string& valueName, const std::string& componentName)
	{
		removeFromContainer(m_FloatProperties, path, valueName, componentName);
	}
	template <>
	void Animation::removePropertySpecialized<void*>(const std::string& path, const std::string& valueName, const std::string& componentName)
	{
		removeFromContainer(m_PointerProperties, path, valueName, componentName);
	}

	template <>
	Property<glm::vec4>* Animation::getPropertySpecialized<glm::vec4>(const std::string& path, const std::string& valueName, const std::string& componentName)
	{
		return findInContainer(m_Vec4Properties, path, valueName, componentName);
	}
	template <>
	Property<glm::vec3>* Animation::getPropertySpecialized<glm::vec3>(const std::string& path, const std::string& valueName, const std::string& componentName)
	{
		return findInContainer(m_Vec3Properties, path, valueName, componentName);
	}
	template <>
	Property<glm::vec2>* Animation::getPropertySpecialized<glm::vec2>(const std::string& path, const std::string& valueName, const std::string& componentName)
	{
		return findInContainer(m_Vec2Properties, path, valueName, componentName);
	}
	template <>
	Property<float>* Animation::getPropertySpecialized<float>(const std::string& path, const std::string& valueName, const std::string& componentName)
	{
		return findInContainer(m_FloatProperties, path, valueName, componentName);
	}
	template <>
	Property<void*>* Animation::getPropertySpecialized<void*>(const std::string& path, const std::string& valueName, const std::string& componentName)
	{
		return findInContainer(m_PointerProperties, path, valueName, componentName);
	}

	template <>
	Property<glm::mat4>* Animation::getPropertySpecialized<glm::mat4>(const std::string& path, const std::string& valueName, const std::string& componentName)
	{
		return nullptr;
	}
	template <>
	Property<Ref<Material>>* Animation::getPropertySpecialized<Ref<Material>>(const std::string& path, const std::string& valueName, const std::string& componentName)
	{
		return nullptr;
	}
	template <>
	Property<Ref<SubTexture>>* Animation::getPropertySpecialized<Ref<SubTexture>>(const std::string& path, const std::string& valueName, const std::string& componentName)
	{
		return nullptr;
	}
	template <>
	Property<uint32_t>* Animation::getPropertySpecialized<uint32_t>(const std::string& path, const std::string& valueName, const std::string& componentName)
	{
		return nullptr;
	}
	template <>
	Property<bool>* Animation::getPropertySpecialized<bool>(const std::string& path, const std::string& valueName, const std::string& componentName)
	{
		return nullptr;
	}
}