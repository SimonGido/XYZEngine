#include "stdafx.h"
#include "BlueprintManager.h"


namespace XYZ {
	namespace Editor {

		static void AddInputArgument(std::string& result, VariableType type, const std::string& name, bool isOutput)
		{
			if (VariableSizeGLSL(type) >= 16)
			{
				result += "in";
			}
			if (isOutput)
			{
				result + "out";
			}
			result += " " + VariableTypeToGLSL(type) + " " + name;
		}

		static void AddOutputArgument(std::string& result, VariableType type, const std::string& name)
		{
			if (VariableSizeGLSL(type) >= 16)
			{
				result += "in";
			}
			result += "out " + VariableTypeToGLSL(type) + " " + name;
		}

		BlueprintManager::BlueprintManager()
		{
			{
				BlueprintFunction func;
				func.Name = "Vec4ToVec3";
				func.Arguments.push_back({ VariableType::Vec4, "input" });
				func.Arguments.push_back({ VariableType::Vec3, "output", true });
				func.OutputType = VariableType::Void;

				func.SourceCode = "vec3 output = input.xyz;\n";

				m_Functions.emplace_back(std::move(func));
			}

			{
				BlueprintFunction func;
				func.Name = "TranslationMatrix";
				func.Arguments.push_back({ VariableType::Vec3, "translation" });
				func.Arguments.push_back({ VariableType::Vec3, "scale" });
				func.Arguments.push_back({ VariableType::Mat4, "transform", true });
				func.OutputType = VariableType::Void;
				func.SourceCode =
					"transform = mat4(\n"
					"	vec4(scale.x, 0.0, 0.0, 0.0),\n"
					"	vec4(0.0, scale.y, 0.0, 0.0),\n"
					"	vec4(0.0, 0.0, scale.z, 0.0),\n"
					"	vec4(translation, 1.0));\n";

				m_Functions.emplace_back(std::move(func));
			}

			{
				BlueprintFunction func;
				func.Name = "SplitTransform";
				func.Arguments.push_back({ VariableType::Mat4, "transform" });

				func.Arguments.push_back({ VariableType::Vec4, "transformRow0", true });
				func.Arguments.push_back({ VariableType::Vec4, "transformRow1", true });
				func.Arguments.push_back({ VariableType::Vec4, "transformRow2", true });

				func.SourceCode =
					"transformRow0 = vec4(transform[0][0], transform[1][0], transform[2][0], transform[3][0]);\n"
					"transformRow1 = vec4(transform[0][1], transform[1][1], transform[2][1], transform[3][1]);\n"
					"transformRow2 = vec4(transform[0][2], transform[1][2], transform[2][2], transform[3][2]);\n";


				m_Functions.emplace_back(std::move(func));
			}

			{
				BlueprintFunction func;
				func.Name = "EntryPoint";

				m_Functions.emplace_back(std::move(func));
			}

		}
		std::string BlueprintManager::GetFunctionString(const std::string& name) const
		{
			const auto& func = *FindFunction(name);
			
			std::string result;
			result += VariableTypeToGLSL(func.OutputType) + " " + func.Name + "(";

			for (size_t i = 0; i < func.Arguments.size(); ++i)
			{
				auto& arg = func.Arguments[i];

				AddInputArgument(result, arg.Type, arg.Name, arg.Output);
				if (i == func.Arguments.size() - 1)
					result += ")";
				else
					result += ",";
			}
			result += "\n";
			result += "{\n";
			result += func.SourceCode;
			result += "\n}\n";

			return result;
		}
		BlueprintFunction* BlueprintManager::FindFunction(const std::string_view name)
		{
			for (auto& func : m_Functions)
			{
				if (func.Name == name)
					return &func;
			}
			return nullptr;
		}
		const BlueprintFunction* BlueprintManager::FindFunction(const std::string_view name) const
		{
			for (auto& func : m_Functions)
			{
				if (func.Name == name)
					return &func;
			}
			return nullptr;
		}
	}
}