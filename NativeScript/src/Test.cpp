#pragma once

#include <XYZ.h>
#include <glm/glm.hpp>

class Testik : public XYZ::ScriptableEntity
{
public:
	virtual void OnUpdate(float dt) override
	{
		//Entity.GetComponent<XYZ::TransformComponent>();
		Entity.GetComponent<XYZ::SpriteRenderer>()->Color = {0,0,0,1 };
	}

};


REGISTERCLASS(Testik)
