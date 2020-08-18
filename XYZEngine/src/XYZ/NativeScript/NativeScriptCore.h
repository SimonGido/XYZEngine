#pragma once

#include "XYZ/Core/Logger.h"

#include <RuntimeObjectSystem/ObjectInterfacePerModule.h>
#include <RuntimeObjectSystem/IObject.h>
#include <RuntimeObjectSystem/RuntimeObjectSystem.h>
#include <RuntimeObjectSystem/ObjectFactorySystem/ObjectFactorySystem.h>
#include <RuntimeCompiler/StdioLogSystem.h>

namespace XYZ {
	enum InterfaceIDEnum
	{
		ID_SCRIPTABLE_ENTITY
	};

	class NativeScriptCore
	{
	public:
		static void Init();
		static void Destroy();

		static void AddModule(const std::string& dir);
		static void RemoveModule(const std::string& dir);
		static void Update(float dt);

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
	};

#define REGISTERMODULE(filepath) NativeScriptCore::AddModule(filepath)
#define REMOVEMODULE(filepath) NativeScriptCore::RemoveModule(filepath)

}