#pragma once
#include "XYZ/Core/Timestep.h"
#include "NativeScriptWrappers.h"

#include <RuntimeObjectSystem/ObjectFactorySystem/ObjectFactorySystem.h>
#include <RuntimeObjectSystem/RuntimeObjectSystem.h>
#include <RuntimeObjectSystem/IObject.h>


namespace XYZ {
	enum class FieldTypes
	{
		INT,
		FLOAT,
		DOUBLE,
		BOOL,
		STRING
	};

	struct ScriptInfoI
	{
		
	};

	template <typename ...Args>
	struct ScriptInfo
	{
		std::tuple<Args...> Data;
	};

}

struct SystemTable
{
	XYZ::Script::NativeAPI NativeAPI;
};

namespace XYZ {

	class NativeScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void Update(Timestep ts);

		static void SetOnRecompileCallback(const std::function<void()>& func);
		static void SetOnReloadCallback(const std::function<void()>& func);

		template <typename T>
		static T* CreateScriptObject()
		{
			IObject* object;
			auto name = TActual<T>::GetTypeNameStatic();
			IObjectConstructor* pCtor = s_pRuntimeObjectSystem->GetObjectFactorySystem()->GetConstructor(name);
			XYZ_ASSERT(pCtor, "Native script class is not registered ", name);

			IObject* pObj = pCtor->Construct();
			pObj->GetInterface(&object);
			XYZ_ASSERT(object, "No scriptable class found");
		
			s_ObjectsCreated.push_back(object);
			return (T*)object;
		}

		static IObject* CreateScriptObject(const std::string& name);		
	};

}