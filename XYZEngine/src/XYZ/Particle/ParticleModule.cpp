#include "stdafx.h"
#include "ParticleModule.h"


namespace XYZ {
	void ParticleModule::LoadFromFile(const std::string& filepath)
	{
	}
	std::string ParticleModule::ElementsToStruct() const
	{
		std::string result;
		result.append("struct " + Name + "\n{\n");
		for (size_t i = 0; i < Elements.size(); ++i)
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
