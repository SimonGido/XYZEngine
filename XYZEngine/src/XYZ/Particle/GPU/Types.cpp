#include "stdafx.h"
#include "Types.h"


namespace XYZ {


	static std::string ShaderVariableTypeToString(ShaderVariableType type)
	{
		switch (type)
		{
		case ShaderVariableType::Float: return "float";
		case ShaderVariableType::Vec2:  return "vec2";
		case ShaderVariableType::Vec3:  return "vec3";
		case ShaderVariableType::Vec4:  return "vec4";
		case ShaderVariableType::Int:   return "int";
		case ShaderVariableType::IVec2: return "ivec2";
		case ShaderVariableType::IVec3: return "ivec3";
		case ShaderVariableType::IVec4: return "ivec4";
		case ShaderVariableType::UInt:  return "uint";
		}
		XYZ_ASSERT(false, "");
		return "none";
	}

	ShaderVariable::ShaderVariable(std::string name, ShaderVariableType type)
		:
		Name(std::move(name)),
		Type(type)
	{
	}

	std::string ShaderVariable::ToString() const
	{
		return ShaderVariableTypeToString(Type) + " " + Name;
	}

	ShaderStruct::ShaderStruct(std::string name, const std::initializer_list<ShaderVariable>& variables)
		:
		Name(std::move(name)),
		Variables(variables)
	{
	}
	ShaderStruct::ShaderStruct(std::string name, std::vector<ShaderVariable> variables)
		:
		Name(std::move(name)),
		Variables(std::move(variables))
	{
	}

	std::string ShaderStruct::ToString() const
	{
		std::string result = "struct " + Name + "{";
		for (const auto& var : Variables)
			result += var.ToString() + ";";
		result += "};";

		return result;
	}

}