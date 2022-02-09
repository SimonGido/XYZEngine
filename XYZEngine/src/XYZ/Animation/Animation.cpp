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
			Reset();
		}
		
		updateProperties(m_CurrentFrame);
		m_CurrentTime += ts;
		m_CurrentFrame = static_cast<uint32_t>(std::floor(m_CurrentTime / m_FrameLength));
	}

	void Animation::Reset()
	{
		resetProperties();
		m_CurrentTime = 0.0f;
		m_CurrentFrame = 0;
	}

	void Animation::UpdateLength()
	{
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

	bool Animation::HasProperty(std::string_view componentName, std::string_view varName, std::string_view path) const
	{
		if (propertyContainerHasVariable(m_Vec4Properties, componentName, varName, path))
			return true;
		if (propertyContainerHasVariable(m_Vec3Properties, componentName, varName, path))
			return true;
		if (propertyContainerHasVariable(m_Vec2Properties, componentName, varName, path))
			return true;
		if (propertyContainerHasVariable(m_FloatProperties, componentName, varName, path))
			return true;
		if (propertyContainerHasVariable(m_PointerProperties, componentName, varName, path))
			return true;
		return false;
	}

	bool Animation::Empty() const
	{
		return m_Vec4Properties.empty() 
			&& m_Vec3Properties.empty() 
			&& m_Vec2Properties.empty() 
			&& m_FloatProperties.empty() 
			&& m_PointerProperties.empty();
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
}