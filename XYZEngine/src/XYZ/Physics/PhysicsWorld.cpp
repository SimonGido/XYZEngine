#include "stdafx.h"
#include "PhysicsWorld.h"

#include "XYZ/Scene/Components.h"

namespace XYZ {


	PhysicsWorld::PhysicsWorld(const glm::vec2& gravity)
		:
		m_Gravity(gravity)
	{
	}
	void PhysicsWorld::Update(Timestep ts)
	{
		
	}
	void PhysicsWorld::AddEntity(SceneEntity entity)
	{
		m_Entities.push_back(entity);
	}
	void PhysicsWorld::RemoveEntity(SceneEntity entity)
	{
		auto it = std::find(m_Entities.begin(), m_Entities.end(), entity);
		if (it != m_Entities.end())
			m_Entities.erase(it);
	}
}