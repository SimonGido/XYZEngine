#pragma once

#include "XYZ/NativeScript/ScriptableEntity.h"

#include <glm/gtx/transform.hpp>

class Testik : public XYZ::ScriptableEntity
{
public:
	virtual void OnCreate() override
	{
		m_SpriteRenderer = API.GetSpriteRenderer(Entity);
		m_TransformComponent = API.GetTransform(Entity);
	}
	virtual void OnUpdate(float dt) override
	{
		m_SpriteRenderer->Color = { 1,0,1,1 };

		m_TransformComponent->Transform[3][1] = m_Position.y;
		m_TransformComponent->Transform[3][0] = m_Position.x;

		m_Position.y += 0.0005;
		m_Position.x += 0.0005;
	}


private:
	XYZ::SpriteRenderer* m_SpriteRenderer;
	XYZ::TransformComponent* m_TransformComponent;

	glm::vec3 m_Position = { 0,0,0 };
};


REGISTERCLASS(Testik)
