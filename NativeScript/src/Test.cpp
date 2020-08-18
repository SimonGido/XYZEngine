#pragma once

#include <XYZ.h>
#include <glm/glm.hpp>

#include "XYZ/NativeScript/Haha.h"

class Testik : public XYZ::ScriptableEntity
{
public:
	virtual void OnUpdate(float dt) override
	{
		PerModuleInterface::g_pSystemTable->SetColor((uint32_t)Entity);
		//XYZ::SetColor((uint32_t)Entity);
		//Entity.GetComponent<XYZ::SpriteRenderer>()->Color = { 0,1,0,1 };
	}
	virtual void Test(XYZ::Scene* scene) override
	{
	}

	virtual bool Lol() override
	{
		return false;
	}
};


REGISTERCLASS(Testik)
