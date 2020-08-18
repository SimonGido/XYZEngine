#pragma once
#include "stdafx.h"
#include "NativeScriptCore.h"


#include "XYZ/NativeScript/ScriptableEntity.h"
#include "XYZ/Scene/SceneManager.h"
#include "Haha.h"
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
		//s_pRuntimeObjectSystem->SetAdditionalLinkOptions("../bin/Debug-windows-x86_64/XYZEngine/XYZEngine.lib");
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



	void NativeScriptCore::Update(float dt)
	{
		if (s_pRuntimeObjectSystem->GetIsCompiledComplete())
		{
			// load module when compile complete
			s_pRuntimeObjectSystem->LoadCompiledModule();
		}

		if (!s_pRuntimeObjectSystem->GetIsCompiling())
		{
			s_pRuntimeObjectSystem->GetFileChangeNotifier()->Update(dt);
		}

		//IObject* kuk;
		//
		//
		//AUDynArray<IObjectConstructor*> constructors;
		//s_pRuntimeObjectSystem->GetObjectFactorySystem()->GetAll(constructors);
		//
		//IObjectConstructor* pCtor = s_pRuntimeObjectSystem->GetObjectFactorySystem()->GetConstructor("Testik");
		//if (pCtor)
		//{
		//	IObject* pObj = pCtor->Construct();
		//	pObj->GetInterface(&kuk);
		//	if (0 == kuk)
		//	{
		//		delete pObj;
		//		s_Logger.LogError("Error - no updateable interface found\n");
		//	}
		//	else
		//	{
		//		ScriptableEntity* testik = (ScriptableEntity*)kuk;
		//		testik->Entity = { 3, SceneManager::Get().GetActive().Raw() };
		//		testik->OnUpdate(dt);
		//
		//		delete testik;
		//	}
		//}
	}

	IRuntimeObjectSystem* NativeScriptCore::GetObjectSystem()
	{
		return s_pRuntimeObjectSystem;
	}
	
}
