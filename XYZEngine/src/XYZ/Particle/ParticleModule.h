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

	// TODO: Maybe you are going to use thath
	template <size_t ElementCount>
	struct ParticleModule
	{
		std::string			  Name;
		ParticleModuleElement Elements[ElementCount];
		bool				  Enabled;

		std::string ToStruct() const;
	};

	template<size_t ElementCount>
	inline std::string ParticleModule<ElementCount>::ToStruct() const
	{
		std::string result;
		result.append("struct " + Name + "\n{\n");
		for (size_t i = 0; i < ElementCount; ++i)
		{
			switch (Elements[i].Type)
			{
			case ParticleShaderDataType::Int:
				result.append("int ");
				break;
			case ParticleShaderDataType::UInt:
				result.append("uint ");
				break;
			case ParticleShaderDataType::Float:
				result.append("float ");
				break;
			case ParticleShaderDataType::Float2:
				result.append("vec2 ");
				break;
			case ParticleShaderDataType::Float3:
				result.append("vec3 ");
				break;
			case ParticleShaderDataType::Float4:
				result.append("vec4 ");
				break;
			}
			result.append(Elements[i].Name + ";\n");
		}
		result.append("\n};\n");
		return result;
	}
}