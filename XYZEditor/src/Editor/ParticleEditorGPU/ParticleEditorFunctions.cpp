#include "stdafx.h"
#include "ParticleEditorFunctions.h"


namespace XYZ {
	namespace Editor {
		std::string ParticleEditorFunction::ToString() const
		{
			std::string result;
			result += VariableTypeToGLSL(OutputType) + " " + Name + "(";


			for (size_t i = 0; i < InputArguments.size(); ++i)
			{
				auto& arg = InputArguments[i];
				if (i == InputArguments.size() - 1)
				{
					result += VariableTypeToGLSL(arg.Type) + " " + arg.Name + ")";
				}
				else
				{
					result += VariableTypeToGLSL(arg.Type) + " " + arg.Name + ",";
				}
			}
			result += "\n";
			result += "{\n";
			result += SourceCode;
			result += "\n}\n";

			return result;
		}
	}
}