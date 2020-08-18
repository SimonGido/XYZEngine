#pragma once
#include <RuntimeObjectSystem/ObjectInterfacePerModule.h>
#include <RuntimeObjectSystem/IObject.h>


#include "XYZ/Scene/Components.h"
#include "XYZ/Core/Core.h"
#include "XYZ/ECS/Entity.h"
#include "NativeScriptCore.h"



namespace XYZ {

	class ScriptableEntity : public TInterface<ID_SCRIPTABLE_ENTITY, IObject>
	{
	public:
		virtual void OnCreate() {};
		virtual void OnUpdate(float dt) {};
		virtual void OnDestroy() {};

		virtual void Test(Scene* scene) {};


		virtual bool Lol() { return true; };
		Entity Entity;
	};

		
}

struct RCCppMainLoopI
{
	virtual void MainLoop() = 0;
};

