#include "stdafx.h"
#include "MonoScriptEngine.h"
#include "XYZ/Core/Application.h"


#include <mono/jit/jit.h>
#include <mono/metadata/loader.h>
#include <mono/metadata/assembly.h>

namespace XYZ {

	static MonoDomain* s_Domain = nullptr;

	static MonoAssembly* s_Assembly = nullptr;

	void MonoScriptEngine::Init(const std::string& assemblyPath)
	{
		s_Domain = mono_jit_init_version("XYZEngine", "v4.0.30319");
		XYZ_ASSERT(s_Domain, "Failed to init jit");

		s_Assembly = mono_domain_assembly_open(s_Domain, assemblyPath.c_str());
		XYZ_ASSERT(s_Assembly, "Failed to open assembly ", assemblyPath);
	}
	void MonoScriptEngine::Destroy()
	{
	}
}