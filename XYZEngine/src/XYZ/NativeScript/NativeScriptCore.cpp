#pragma once
#include "stdafx.h"
#include "NativeScriptCore.h"

#include "Haha.h"

#include <RuntimeObjectSystem/ObjectInterfacePerModule.h>

namespace XYZ {


	IRuntimeObjectSystem* NativeScriptCore::s_pRuntimeObjectSystem;
	StdioLogSystem        NativeScriptCore::s_Logger;
	SystemTable			  NativeScriptCore::s_SystemTable;

	void NativeScriptCore::Init()
	{
		s_pRuntimeObjectSystem = new RuntimeObjectSystem;
		if (!s_pRuntimeObjectSystem->Initialise(&s_Logger, &s_SystemTable))
		{
			delete s_pRuntimeObjectSystem;
			s_pRuntimeObjectSystem = NULL;
		}

		s_SystemTable.SetColor = &SetColor;
		s_pRuntimeObjectSystem->AddIncludeDir("../XYZEngine/src/");
		s_pRuntimeObjectSystem->AddIncludeDir("../XYZEngine/vendor/glm/");
		s_pRuntimeObjectSystem->AddIncludeDir("../XYZEngine/vendor/RCC/");

		s_pRuntimeObjectSystem->SetAdditionalCompileOptions("/std:c++17");
		s_pRuntimeObjectSystem->SetAdditionalLinkOptions("../bin/Debug-windows-x86_64/XYZEngine/XYZEngine.lib");
	}

	void NativeScriptCore::Destroy()
	{
		s_pRuntimeObjectSystem->CleanObjectFiles();
		delete s_pRuntimeObjectSystem;
	}

	void NativeScriptCore::SetScene(Scene* scene)
	{
		PerModuleInterface::g_pSystemTable->CurrentScene = scene;
	}

	void NativeScriptCore::AddModule(const std::string& dir)
	{
		XYZ_LOG_INFO("Adding module ", dir);
	}

	void NativeScriptCore::RemoveModule(const std::string& dir)
	{
		XYZ_LOG_INFO("Removing module ", dir);
	}


	bool NativeScriptCore::Update(float dt)
	{
		if (s_pRuntimeObjectSystem->GetIsCompiledComplete())
		{
			// load module when compile complete
			s_pRuntimeObjectSystem->LoadCompiledModule();
			return true;
		}

		if (!s_pRuntimeObjectSystem->GetIsCompiling())
		{
			s_pRuntimeObjectSystem->GetFileChangeNotifier()->Update(dt);
		}
		return false;
	}
}
