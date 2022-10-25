#pragma once
#include "XYZ/Utils/VariableTypes.h"

#include <string>

namespace XYZ {
	namespace Editor {

		struct InputArgument
		{
			VariableType Type;
			std::string  Name;
		};

		struct ParticleEditorFunction
		{
			std::string Name;
			std::string SourceCode;

			std::vector<InputArgument> InputArguments;
			VariableType			   OutputType;

			std::string ToString() const;
		};
	}
}