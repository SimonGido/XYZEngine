#include "stdafx.h"
#include "Blueprint.h"

#include "XYZ/Utils/Math/Math.h"

#include "XYZ/Utils/FileSystem.h"

namespace XYZ {
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


	void Blueprint::AddStruct(const BlueprintStruct& str)
	{
		m_Structs.push_back(str);
		m_BuildRequired = true;
	}

	void Blueprint::AddFunction(const BlueprintFunction& func)
	{
		m_Functions.push_back(func);
		m_BuildRequired = true;
	}

	void Blueprint::SetFunctionSequence(const BlueprintFunctionSequence& seq)
	{
		m_Sequence = seq;
		m_BuildRequired = true;
	}
	void Blueprint::Rebuild()
	{
		m_SourceCode.clear();
		for (auto& str : m_Structs)
		{
			addStructDefinition(str);
		}
		for (auto& func : m_Functions)
		{
			addFunctionDefinition(func);
		}
		addFunctionSequence(m_Sequence);
		m_BuildRequired = false;

		FileSystem::WriteFile("Blueprint.glsl", m_SourceCode);
	}
	void Blueprint::addStructDefinition(const BlueprintStruct& str)
	{
		uint32_t offset = 0;
		if (!str.Variables.empty())
		{
			m_SourceCode += fmt::format("struct {}", str.Name) + "\n{\n";
			for (auto& var : str.Variables)
			{
				if (var.IsArray)
				{
					m_SourceCode += fmt::format("	{} {}[];\n", VariableTypeToGLSL(var.Type), var.Name);
				}
				else
				{
					m_SourceCode += fmt::format("	{} {};\n", VariableTypeToGLSL(var.Type), var.Name);
				}
				offset += var.Size;
			}

			uint32_t paddingSize = Math::RoundUp(offset, 16) - offset;
			if (paddingSize != 0)
			{
				m_SourceCode += fmt::format("	uint Padding[{}];\n", paddingSize / sizeof(uint32_t));
			}
			m_SourceCode += "};\n";
		}
	}

	void Blueprint::addFunctionDefinition(const BlueprintFunction& func)
	{
		addBeginFunction(func);

		m_SourceCode += func.SourceCode;
		m_SourceCode += "\n}\n";
	}

	void Blueprint::addFunctionSequence(const BlueprintFunctionSequence& seq)
	{
		addBeginFunction(seq.EntryPoint);

		for (auto& call : seq.FunctionCalls)
		{
			addFunctionCall(call);
		}

		m_SourceCode += "\n}\n";
	}

	void Blueprint::addFunctionCall(const BlueprintFunctionCall& call)
	{
		m_SourceCode += call.Name + "(";
		for (size_t i = 0; i < call.Arguments.size(); ++i)
		{
			auto& arg = call.Arguments[i];
			m_SourceCode += arg.Name;
			if (i != call.Arguments.size() - 1)
				m_SourceCode += ",";
		}
		m_SourceCode += ");\n";
	}

	void Blueprint::addBeginFunction(const BlueprintFunction& func)
	{
		m_SourceCode += VariableTypeToGLSL(func.OutputType) + " " + func.Name + "(";

		for (size_t i = 0; i < func.Arguments.size(); ++i)
		{
			auto& arg = func.Arguments[i];
			if (arg.Output)
				AddOutputArgument(m_SourceCode, arg.Type, arg.Name);
			else
				AddInputArgument(m_SourceCode, arg.Type, arg.Name, arg.Output);
			
			if (i == func.Arguments.size() - 1)
				m_SourceCode += ")";
			else
				m_SourceCode += ",";
		}
		m_SourceCode += "\n";
		m_SourceCode += "{\n";
	}

}