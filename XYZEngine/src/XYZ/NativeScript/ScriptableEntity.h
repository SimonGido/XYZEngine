#pragma once
#include <RuntimeObjectSystem/ObjectInterfacePerModule.h>
#include <RuntimeObjectSystem/ObjectFactorySystem/ObjectFactorySystem.h>
#include <RuntimeObjectSystem/IObject.h>



#include "NativeScriptWrappers.h"
#include "NativeScriptEngine.h"

namespace XYZ {

	class ScriptableEntity : public TInterface<ID_SCRIPTABLE_ENTITY, IObject>
	{
	public:
		ScriptableEntity()
			:
			API(PerModuleInterface::g_pSystemTable->NativeAPI)
		{}

		virtual ~ScriptableEntity() {};

		virtual void OnCreate() {};
		virtual void OnUpdate(Timestep ts) {};
		virtual void OnDestroy() {};

		Entity Entity;
		const XYZ::Script::NativeAPI& API;
	};

		
}

struct RCCppMainLoopI
{
	virtual void MainLoop() = 0;
};

