#pragma once
#include "XYZ/Scene/SceneEntity.h"

#include <glm/glm.hpp>

namespace XYZ {

	class PhysicsWorld
	{
	public:
		PhysicsWorld(const glm::vec2& gravity);

		void Update(Timestep ts);

		uint32_t CreatePhysicsObject(uint32_t id);


	private:
		DynamicTree m_Tree;
		glm::vec2 m_Gravity;
	};
}