#pragma once
#include <XYZ.h>


class Testik : public XYZ::ScriptableEntity
{
public:
	virtual void OnUpdate(float dt) override
	{
		XYZ::TransformComponent transform;
		NATIVE SetColor({ 0,1,1,1 }, Entity);
	}
};


REGISTERCLASS(Testik)
