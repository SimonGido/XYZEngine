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

		void AddLayer(const std::string& name, uint8_t id);


		ScopedLock<b2World>		GetWorld();
		ScopedLockRead<b2World>	GetWorldRead() const;

	private:
		void synchronize();

	private:
		b2World					  m_World;
		mutable std::shared_mutex m_Mutex;

		struct Layer
		{
			std::string Name;
			uint8_t		ID;
		};
		std::array<Layer, 16> m_Layers;
	};
}