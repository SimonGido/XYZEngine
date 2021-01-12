#include "stdafx.h"
#include "MonoScriptEngine.h"

#include <mono/jit/jit.h>
#include <mono/metadata/loader.h>
#include <mono/metadata/assembly.h>

namespace XYZ {


	void MonoScriptEngine::Init()
	{
		MonoDomain* domain;
		domain = mono_jit_init_version("myapp", "v4.0.30319");

		MonoAssembly * assembly;

		assembly = mono_domain_assembly_open(domain, "file.exe");
		if (!assembly)
		{
		}
	}
}