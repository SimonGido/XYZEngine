#pragma once
#include "XYZ/Utils/DataStructures/ThreadPass.h"
#include "XYZ/Core/Timestep.h"

#include <box2d/box2d.h>

#include <glm/glm.hpp>

namespace XYZ {

	class PhysicsWorld2D
	{
	public:
		PhysicsWorld2D(const glm::vec2& gravity);

		void Step(Timestep ts);

		ScopedLock<b2World>		GetWorld();
		ScopedLockRead<b2World>	GetWorldRead() const;

	private:
		b2World					  m_World;
		mutable std::shared_mutex m_Mutex;
	};
}