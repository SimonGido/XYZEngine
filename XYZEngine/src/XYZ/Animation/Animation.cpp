#include "stdafx.h"
#include "Animation.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Scene/Components.h"

#include <glm/gtx/matrix_interpolation.hpp>

namespace XYZ {

	Animation::Animation()
		:
		m_NumFrames(0),
		m_Frequency(0),
		m_Repeat(true)
	{
		SetFrequency(60);
	}

	Animation::~Animation()
	{
	}


	void Animation::SetFrequency(uint32_t framesPerSecond)
	{
		m_Frequency = framesPerSecond;
		m_FrameLength = 1.0f / m_Frequency;
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

	size_t Animation::GetPropertyCount() const
	{
		return m_Vec4Properties.size()
			 + m_Vec3Properties.size()
			 + m_Vec2Properties.size()
			 + m_FloatProperties.size()
			 + m_PointerProperties.size();
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