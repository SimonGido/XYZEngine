#include "stdafx.h"
#include "ParticleSystemLayout.h"

namespace XYZ {

	static uint32_t ParticleVariableTypSize(ParticleVariableType type)
	{
		switch (type)
		{
		case XYZ::ParticleVariableType::None:
			XYZ_ASSERT(false, "Invalid type");
			return 0;
		case XYZ::ParticleVariableType::Float:
			return 4;
		case XYZ::ParticleVariableType::Vec2:
			return 8;
		case XYZ::ParticleVariableType::Vec3:
			return 12;
		case XYZ::ParticleVariableType::Vec4:
			return 16;
		case XYZ::ParticleVariableType::Int:
			return 4;
		case XYZ::ParticleVariableType::IVec2:
			return 8;
		case XYZ::ParticleVariableType::IVec3:
			return 12;
		case XYZ::ParticleVariableType::IVec4:
			return 16;
		case XYZ::ParticleVariableType::Bool: // Bool has 4 bytes size in GLSL
			return 4;
		default:
			break;
		}
		return 0;
	}

	ParticleSystemLayout::ParticleSystemLayout(const std::vector<ParticleVariableInit>& particleLayout)
	{
		m_Variables.resize(particleLayout.size());


		uint32_t offset = 0;
		for (size_t i = 0; i < m_Variables.size(); ++i)
		{
			m_Variables[i].Name = particleLayout[i].first;
			m_Variables[i].Type = particleLayout[i].second;
			m_Variables[i].Size = ParticleVariableTypSize(m_Variables[i].Type);
			m_Variables[i].Offset = offset;
			offset += m_Variables[i].Size;
		}
		m_Stride = offset;
	}
	uint32_t ParticleSystemLayout::GetVariableOffset(const std::string_view name) const
	{
		for (const auto& var : m_Variables)
		{
			if (var.Name == name)
				return var.Offset;
		}
		XYZ_ASSERT(false, "Variable does not exist");
		return 0;
	}
	uint32_t ParticleSystemLayout::GetStride() const
	{
		return m_Stride;
	}
}