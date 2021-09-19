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
		m_FrameLength(0.05f),
		m_Repeat(true)
	{
	}

	Animation::~Animation()
	{
		if (m_ActiveScene.Raw())
		{
			m_ActiveScene->GetECS().RemoveOnConstruction<Animation>(&Animation::updatePropertyReferences, this);
			m_ActiveScene->GetECS().RemoveOnDestruction<Animation>(&Animation::updatePropertyReferences, this);
		}
	}

	void Animation::SetActiveScene(const Ref<Scene>& scene)
	{
		if (m_ActiveScene.Raw() != scene.Raw())
		{
			if (m_ActiveScene.Raw())
			{
				clearProperties();
				m_ActiveScene->GetECS().RemoveOnConstruction<Animation>(&Animation::updatePropertyReferences, this);
				m_ActiveScene->GetECS().RemoveOnDestruction<Animation>(&Animation::updatePropertyReferences, this);
			}
			m_ActiveScene = scene;
			m_ActiveScene->GetECS().AddOnConstruction<Animation>(&Animation::updatePropertyReferences, this);
			m_ActiveScene->GetECS().AddOnDestruction<Animation>(&Animation::updatePropertyReferences, this);
		}
	}

	void Animation::Update(Timestep ts)
	{
		if (m_CurrentFrame >= m_NumFrames)
		{
			if (!m_Repeat)
				return;

			m_CurrentTime = 0.0f;
			m_CurrentFrame = 0;
		}

		m_CurrentTime += ts;
		m_CurrentFrame = static_cast<uint32_t>(std::floor(m_CurrentTime / m_FrameLength));
	}

	void Animation::UpdateLength()
	{

	}

	void Animation::SetCurrentFrame(uint32_t frame)
	{
		m_CurrentFrame = std::min(m_NumFrames, frame);
		m_CurrentTime = m_CurrentFrame * m_FrameLength;
	}

	void Animation::updatePropertyReferences()
	{
		for (auto& prop : m_Vec4Properties)
			prop.SetReference();
		for (auto& prop : m_Vec3Properties)
			prop.SetReference();
		for (auto& prop : m_Vec2Properties)
			prop.SetReference();
		for (auto& prop : m_FloatProperties)
			prop.SetReference();
		for (auto& prop : m_PointerProperties)
			prop.SetReference();
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

	template <>
	void Animation::removePropertySpecialized<glm::vec4>(const SceneEntity& entity, const std::string& valueName, const std::string& componentName)
	{
		removeFromContainer(m_Vec4Properties, entity, valueName, componentName);
	}
	template <>
	void Animation::removePropertySpecialized<glm::vec3>(const SceneEntity& entity, const std::string& valueName, const std::string& componentName)
	{
		removeFromContainer(m_Vec3Properties, entity, valueName, componentName);
	}
	template <>
	void Animation::removePropertySpecialized<glm::vec2>(const SceneEntity& entity, const std::string& valueName, const std::string& componentName)
	{
		removeFromContainer(m_Vec2Properties, entity, valueName, componentName);
	}
	template <>
	void Animation::removePropertySpecialized<float>(const SceneEntity& entity, const std::string& valueName, const std::string& componentName)
	{
		removeFromContainer(m_FloatProperties, entity, valueName, componentName);
	}
	template <>
	void Animation::removePropertySpecialized<void*>(const SceneEntity& entity, const std::string& valueName, const std::string& componentName)
	{
		removeFromContainer(m_PointerProperties, entity, valueName, componentName);
	}
}