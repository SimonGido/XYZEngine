#include "stdafx.h"
#include "ParticleSystemLayout.h"

#include "XYZ/Utils/Math/Math.h"

namespace XYZ {

	
	ParticleSystemLayout::ParticleSystemLayout(std::string name, const std::vector<ParticleVariableInit>& particleLayout, bool round)
		:
		m_Name(std::move(name))
	{
		m_Variables.resize(particleLayout.size());


		uint32_t offset = 0;
		for (size_t i = 0; i < m_Variables.size(); ++i)
		{
			m_Variables[i].Name = particleLayout[i].first;
			m_Variables[i].Type = particleLayout[i].second;
			m_Variables[i].Size = VariableSizeGLSL(m_Variables[i].Type);
			m_Variables[i].Offset = offset;
			offset += m_Variables[i].Size;
		}
		m_Stride = Math::RoundUp(offset, 16);
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