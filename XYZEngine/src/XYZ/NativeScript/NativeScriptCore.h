#pragma once

#include "XYZ/Core/Logger.h"

#include <RuntimeObjectSystem/ObjectInterfacePerModule.h>
#include <RuntimeObjectSystem/IObject.h>
#include <RuntimeObjectSystem/RuntimeObjectSystem.h>
#include <RuntimeObjectSystem/ObjectFactorySystem/ObjectFactorySystem.h>
#include <RuntimeCompiler/StdioLogSystem.h>

#include "XYZ/Scene/Scene.h"

struct RCCppMainLoopI;
struct SystemTable
{
	XYZ::Scene* CurrentScene = nullptr;
	RCCppMainLoopI* MainLoop = nullptr;
	std::function<void(uint32_t)> SetColor;
};

namespace XYZ {
	enum InterfaceIDEnum
	{
		ID_SCRIPTABLE_ENTITY,
		IID_IRCCPP_MAIN_LOOP
	};
	
	class NativeScriptCore
	{
	public:
		static void Init();
		static void Destroy();

		static void SetScene(Scene* scene);
		static void AddModule(const std::string& dir);
		static void RemoveModule(const std::string& dir);
		static void Update(float dt);

		static IRuntimeObjectSystem* GetObjectSystem();
		
		template <typename T>
		static T* GetScriptClass()
		{
			IObject* object;
			auto name = TActual<T>::GetTypeNameStatic();
			IObjectConstructor* pCtor = s_pRuntimeObjectSystem->GetObjectFactorySystem()->GetConstructor(name);
			XYZ_ASSERT(pCtor, "Native script class is not registered ", name);
			
			IObject* pObj = pCtor->Construct();
			pObj->GetInterface(&object);
			if (!object)
			{
				delete pObj;
				XYZ_ASSERT(false,"No interface found");
			}
			else
			{
				T* scriptable = (T*)object;
				return scriptable;
			}
		}

	private:
		static IRuntimeObjectSystem* s_pRuntimeObjectSystem;
		static StdioLogSystem        s_Logger;
		static SystemTable			 s_SystemTable;
	};


	

#define REGISTERMODULE(filepath) NativeScriptCore::AddModule(filepath)
#define REMOVEMODULE(filepath) NativeScriptCore::RemoveModule(filepath)

}