#include "stdafx.h"
#include "BlueprintManager.h"


namespace XYZ {
	namespace Editor {

	

		BlueprintManager::BlueprintManager(VariableManager* manager)
			:
			m_VariableManager(manager)
		{
			{
				BlueprintFunction func;
				func.Name = "Vec4ToVec3";
				func.Arguments.push_back({ manager->GetVariable("vec4"), "input"});
				func.Arguments.push_back({ manager->GetVariable("vec3"), "output", true});

				func.SourceCode = "vec3 output = input.xyz;\n";

				m_Functions.emplace_back(std::move(func));
			}

			{
				BlueprintFunction func;
				func.Name = "TranslationMatrix";
				func.Arguments.push_back({ manager->GetVariable("vec3"), "translation" });
				func.Arguments.push_back({ manager->GetVariable("vec3"), "scale" });
				func.Arguments.push_back({ manager->GetVariable("mat4"), "transform", true });

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
				func.Arguments.push_back({ manager->GetVariable("mat4"), "transform" });

				func.Arguments.push_back({ manager->GetVariable("vec4"), "transformRow0", true });
				func.Arguments.push_back({ manager->GetVariable("vec4"), "transformRow1", true });
				func.Arguments.push_back({ manager->GetVariable("vec4"), "transformRow2", true });

				func.SourceCode =
					"transformRow0 = vec4(transform[0][0], transform[1][0], transform[2][0], transform[3][0]);\n"
					"transformRow1 = vec4(transform[0][1], transform[1][1], transform[2][1], transform[3][1]);\n"
					"transformRow2 = vec4(transform[0][2], transform[1][2], transform[2][2], transform[3][2]);\n";


				m_Functions.emplace_back(std::move(func));
			}

			{
				BlueprintFunction func;
				func.Name = "main";
				func.Entry = true;
				func.Arguments.push_back({ manager->GetVariable("ivec3"), "workgroups" });
				m_Functions.emplace_back(std::move(func));
			}

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