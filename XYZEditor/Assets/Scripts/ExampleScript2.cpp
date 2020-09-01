#pragma once

#include "XYZ/NativeScript/ScriptableEntity.h"

#include <glm/gtx/transform.hpp>

class Testik2 : public XYZ::ScriptableEntity
{
public:
	virtual void OnCreate() override
	{
		m_SpriteRenderer = API.GetSpriteRenderer(Entity);
		m_TransformComponent = API.GetTransform(Entity);
		m_Position = { m_TransformComponent->Transform[3][0],
					   m_TransformComponent->Transform[3][1],
					   m_TransformComponent->Transform[3][2] };

	}
	virtual void OnUpdate(XYZ::Timestep ts) override
	{
		m_SpriteRenderer->Color = { 0,1,1,1 };

		m_TransformComponent->Transform[3][0] = m_Position.x;
		m_TransformComponent->Transform[3][1] = m_Position.y;

		m_Position.y -= 0.0005;
		m_Position.x -= 0.0005;
	}
	
private:	
	XYZ::SpriteRenderer* m_SpriteRenderer;
	XYZ::TransformComponent* m_TransformComponent;
	glm::vec3 m_Position;
};


REGISTERCLASS(Testik2)
