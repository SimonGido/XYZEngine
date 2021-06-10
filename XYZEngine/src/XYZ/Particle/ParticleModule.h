#pragma once

#include <glm/glm.hpp>

#include <stdint.h>

namespace XYZ {
	
	
	enum class ParticleShaderDataType
	{
		Int, UInt, Float, Float2, Float3, Float4
	};
	
	struct ParticleModuleElement
	{
		std::string			   Name;
		ParticleShaderDataType Type;
	};

	struct ParticleModule
	{
		std::string						   Name;
		std::string						   SourceCode;
		std::vector<ParticleModuleElement> Elements;
		bool							   Enabled;

		void LoadFromFile(const std::string& filepath);
		
		std::string ElementsToStruct() const;
	};
}