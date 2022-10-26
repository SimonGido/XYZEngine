#pragma once
#include "XYZ/Core/Core.h"
#include "XYZ/Core/Assert.h"

#include <string>

namespace XYZ {
	enum class VariableType
	{
		None,
		Function,
		Void,
		Float,
		Vec2,
		Vec3,
		Vec4,

		Int,
		IVec2,
		IVec3,
		IVec4,

		Bool,
		Mat4,
		Num
	};

	struct VariablePair
	{
		std::string  TypeString;
		VariableType Type;
	};

	static std::string VariableTypeToGLSL(VariableType type)
	{
		switch (type)
		{
		case VariableType::None:  break;
		case VariableType::Void:  return "void";
		case VariableType::Float: return "float";
		case VariableType::Vec2:  return "vec2";
		case VariableType::Vec3:  return "vec3";
		case VariableType::Vec4:  return "vec4";

		case VariableType::Int:	  return "int";
		case VariableType::IVec2: return "ivec2";
		case VariableType::IVec3: return "ivec3";
		case VariableType::IVec4: return "ivec4";
		case VariableType::Bool:  return "bool";
		case VariableType::Mat4:  return "mat4";
		};

		XYZ_ASSERT(false, "Invalid type");
		return std::string();
	}


	static uint32_t VariableSizeGLSL(VariableType type)
	{
		switch (type)
		{
		case VariableType::None:
			XYZ_ASSERT(false, "Invalid type");
			return 0;
		case VariableType::Void:
			return 0;
		case VariableType::Float:
			return 4;
		case VariableType::Vec2:
			return 8;
		case VariableType::Vec3:
			return 12;
		case VariableType::Vec4:
			return 16;
		case VariableType::Int:
			return 4;
		case VariableType::IVec2:
			return 8;
		case VariableType::IVec3:
			return 12;
		case VariableType::IVec4:
			return 16;
		case VariableType::Bool: // Bool has 4 bytes size in GLSL
			return 4;
		case VariableType::Mat4:
			return 4 * 4 * 4;
		default:
			break;
		}
		return 0;
	}

	static const std::array<VariablePair, ToUnderlying(VariableType::Num)>& VariableTypes()
	{
		static std::array<VariablePair, ToUnderlying(VariableType::Num)> variableTypes = {
			VariablePair{"None",	 VariableType::None},
			VariablePair{"Function", VariableType::None},
			VariablePair{"Void",	 VariableType::None},
			VariablePair{"Float",	 VariableType::None},
			VariablePair{"Vec2",	 VariableType::None},
			VariablePair{"Vec3",	 VariableType::None},
			VariablePair{"Vec4",	 VariableType::None},
			VariablePair{"Int",		 VariableType::None},
			VariablePair{"IVec2",	 VariableType::None},
			VariablePair{"IVec3",	 VariableType::None},
			VariablePair{"IVec4",	 VariableType::None},
			VariablePair{"Bool",	 VariableType::None},
			VariablePair{"Mat4",	 VariableType::None}
		};
		return variableTypes;
	}

	static const std::string& VariableTypeToImGui(VariableType type)
	{
		return VariableTypes()[ToUnderlying(type)].TypeString;
	}
}