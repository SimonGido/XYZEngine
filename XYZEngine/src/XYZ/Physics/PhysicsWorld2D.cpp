#include "stdafx.h"
#include "PhysicsWorld2D.h"

#include "XYZ/Core/Application.h"
#include "XYZ/Debug/Profiler.h"

namespace XYZ {
	PhysicsWorld2D::PhysicsWorld2D(const glm::vec2& gravity)
		:
		m_World({ gravity.x, gravity.y })
	{
	}
	void PhysicsWorld2D::Step(Timestep ts)
	{
		XYZ_PROFILE_FUNC("PhysicsWorld2D::Step");
		synchronize();
		Application::Get().GetThreadPool().PushJob<void>([this, ts]() {
			XYZ_PROFILE_FUNC("PhysicsWorld2D::Step Job");
			std::scoped_lock lock(m_Mutex);
			int32_t velocityIterations = 6;
			int32_t positionIterations = 2;
			m_World.Step(ts.GetSeconds(), velocityIterations, positionIterations);
		});	
	}
	ScopedLock<b2World> PhysicsWorld2D::GetWorld()
	{
		return ScopedLock<b2World>(&m_Mutex, m_World);
	}
	ScopedLockRead<b2World> PhysicsWorld2D::GetWorldRead() const
	{
		return ScopedLockRead<b2World>(&m_Mutex, m_World);
	}
	void PhysicsWorld2D::synchronize()
	{
		XYZ_PROFILE_FUNC("PhysicsWorld2D::synchronize");
		std::scoped_lock lock(m_Mutex);
	}
}