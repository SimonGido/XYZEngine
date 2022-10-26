#include "stdafx.h"
#include "BlueprintVariableType.h"

namespace XYZ {

	VariableManager::VariableManager()
	{
		RegisterVariable("function", 0	);
		RegisterVariable("void",	 0	);
		RegisterVariable("float",	 4	);
		RegisterVariable("vec2",	 8	);
		RegisterVariable("vec3",	 12	);
		RegisterVariable("vec4",	 16	);
		RegisterVariable("int",		 4	);
		RegisterVariable("ivec2",	 8	);
		RegisterVariable("ivec3",	 12	);
		RegisterVariable("ivec4",	 16	);
		RegisterVariable("bool",	 4	);
		RegisterVariable("mat4",	 64	);
	}

	void VariableManager::RegisterVariable(const std::string& name, uint32_t size)
	{
		XYZ_ASSERT(m_Variables.find(name) == m_Variables.end(), "Variable already exists");
		
		m_Variables[name] = { name, size };
	}
}