#pragma once
#include "XYZ/Core/Core.h"
#include "XYZ/Core/Assert.h"

#include <string>

namespace XYZ {
	enum class VariableType
	{
		None,
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


	static std::string VariableTypeToGLSL(VariableType type)
	{

		switch (type)
		{
		case VariableType::None:  break;
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

	static const std::string& VariableTypeToImGui(VariableType type)
	{
		static std::array<std::string, ToUnderlying(VariableType::Num)> variableNames = {
			"None",
			"Float",
			"Vec2",
			"Vec3",
			"Vec4",
			"Int",
			"IVec2",
			"IVec3",
			"IVec4",
			"Bool",
			"Mat4"
		};
		return variableNames[ToUnderlying(type)];
	}
}