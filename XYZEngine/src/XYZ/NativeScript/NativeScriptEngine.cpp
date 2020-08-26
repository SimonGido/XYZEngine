#include "stdafx.h"
#include "NativeScriptEngine.h"

#include "NativeScriptLogger.h"

namespace XYZ {



	static IRuntimeObjectSystem* s_pRuntimeObjectSystem;
	static NativeScriptLogger    s_Logger;
	static SystemTable			 s_SystemTable;
	static std::function<void()> s_OnRecompileCallback;
	static std::function<void()> s_OnReloadCallback;

	static std::vector<IObject*> s_ObjectsCreated;
	


	void NativeScriptEngine::Init()
	{
		s_pRuntimeObjectSystem = new RuntimeObjectSystem;
		if (!s_pRuntimeObjectSystem->Initialise(&s_Logger, &s_SystemTable))
		{
			delete s_pRuntimeObjectSystem;
			s_pRuntimeObjectSystem = NULL;
		}

		s_pRuntimeObjectSystem->AddIncludeDir("../XYZEngine/src/");
		s_pRuntimeObjectSystem->AddIncludeDir("../XYZEngine/vendor/glm/");
		s_pRuntimeObjectSystem->AddIncludeDir("../XYZEngine/vendor/RCC/");
		s_pRuntimeObjectSystem->SetAdditionalCompileOptions("/std:c++17");
	}

	void NativeScriptEngine::Shutdown()
	{
		s_pRuntimeObjectSystem->CleanObjectFiles();
		delete s_pRuntimeObjectSystem;
	}

	void NativeScriptEngine::Update(Timestep ts)
	{
		if (s_pRuntimeObjectSystem->GetIsCompiledComplete())
		{
			// Call before recompiling
			if (s_OnRecompileCallback)
				s_OnRecompileCallback();

			// Delete old objects that were created
			for (auto object : s_ObjectsCreated)
				delete object;
			s_ObjectsCreated.clear();

			// load module when compile complete
			s_pRuntimeObjectSystem->LoadCompiledModule();				

			// Call after module is loaded
			if (s_OnReloadCallback)
				s_OnReloadCallback();
		}

		if (!s_pRuntimeObjectSystem->GetIsCompiling())
		{
			s_pRuntimeObjectSystem->GetFileChangeNotifier()->Update(ts);
		}
	}

	void NativeScriptEngine::SetOnRecompileCallback(const std::function<void()>& func)
	{
		s_OnRecompileCallback = func;
	}

	void NativeScriptEngine::SetOnReloadCallback(const std::function<void()>& func)
	{
		s_OnReloadCallback = func;
	}

	IObject* NativeScriptEngine::CreateScriptObject(const std::string& name)
	{
		IObject* object;
		IObjectConstructor* pCtor = s_pRuntimeObjectSystem->GetObjectFactorySystem()->GetConstructor(name.c_str());
		XYZ_ASSERT(pCtor, "Native script class is not registered ", name);

		IObject* pObj = pCtor->Construct();
		pObj->GetInterface(&object);
		XYZ_ASSERT(object, "No scriptable class found");
		s_ObjectsCreated.push_back(object);
		return object;
	}

	
}