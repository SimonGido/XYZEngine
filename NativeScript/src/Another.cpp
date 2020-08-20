#pragma once


#include "XYZ/NativeScript/ScriptableEntity.h"


class Another : public XYZ::ScriptableEntity
{
public:
	virtual void OnCreate() override
	{
		m_SpriteRenderer = API.GetSpriteRenderer(Entity);
	}
	virtual void OnUpdate(float dt) override
	{
		m_SpriteRenderer->Color = { 0,1,0,1 };
	}


private:
	XYZ::SpriteRenderer* m_SpriteRenderer;
};


REGISTERCLASS(Another)
