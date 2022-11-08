#pragma once
#include "XYZ/Scene/BlueprintVariableType.h"

namespace XYZ {


	struct ParticleVariable
	{
		std::string			 Name;
		VariableType		 Type;
		bool				 IsArray = false;
		uint32_t			 Offset = 0;
	};


	struct ParticleSystemLayout
	{
		using ParticleVariableInit = std::pair<std::string, VariableType>;

		ParticleSystemLayout(std::string name, const std::vector<ParticleVariableInit>& particleLayout, bool round = true);

		uint32_t GetVariableOffset(const std::string_view name) const;

		const std::string& GetName() const { return m_Name; }
		const std::vector<ParticleVariable>& GetVariables() const { return m_Variables; }
		uint32_t							 GetStride() const;
	private:
		std::string					  m_Name;
		std::vector<ParticleVariable> m_Variables;
		uint32_t					  m_Stride;
	};
}