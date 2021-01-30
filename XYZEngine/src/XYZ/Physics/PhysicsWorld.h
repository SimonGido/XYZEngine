#pragma once
#include "XYZ/Scene/SceneEntity.h"

#include <glm/glm.hpp>

namespace XYZ {

	class PhysicsWorld
	{
	public:
		PhysicsWorld(const glm::vec2& gravity);

		void Update(Timestep ts);

		void AddEntity(SceneEntity entity);
		void RemoveEntity(SceneEntity entity);

	private:
		float calculateTorque(const glm::vec2& force);

	private:
		std::vector<SceneEntity> m_Entities;

		glm::vec2 m_Gravity;
	};
}