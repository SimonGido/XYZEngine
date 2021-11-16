#include "stdafx.h"
#include "PhysicsWorld2D.h"

#include "XYZ/Core/Application.h"
#include "XYZ/Debug/Profiler.h"

namespace XYZ {
	PhysicsWorld2D::PhysicsWorld2D(const glm::vec2& gravity)
		:
		m_World({ gravity.x, gravity.y })
	{
		m_Layers[DefaultLayer] = { "Default", DefaultLayer, {} };
		m_Layers[ParticleLayer] = { "Particle", ParticleLayer, {} };

		m_Layers[DefaultLayer].m_CollisionMask.set(DefaultLayer, true);
		m_Layers[DefaultLayer].m_CollisionMask.set(ParticleLayer, true);

		m_Layers[ParticleLayer].m_CollisionMask.set(DefaultLayer, true);
		m_Layers[ParticleLayer].m_CollisionMask.set(ParticleLayer, false);
	}
	void PhysicsWorld2D::Step(Timestep ts)
	{
		XYZ_PROFILE_FUNC("PhysicsWorld2D::Step");
		synchronize();
		Application::Get().GetThreadPool().PushJob<void>([this, ts]() {
			XYZ_PROFILE_FUNC("PhysicsWorld2D::Step Job");
			std::scoped_lock lock(m_Mutex);
			const int32_t velocityIterations = 6;
			const int32_t positionIterations = 2;
			m_World.Step(ts.GetSeconds(), velocityIterations, positionIterations);
		});	
	}
	void PhysicsWorld2D::SetLayer(const std::string& name, uint32_t index, const CollisionMask& mask)
	{
		XYZ_ASSERT(index >= sc_NumDefaultLayers, "Attempting to change default collision layer");
		m_Layers.at(index) = { name, index, mask };
	}
	const PhysicsWorld2D::Layer& PhysicsWorld2D::GetLayer(std::string_view name) const
	{
		for (const auto& layer : m_Layers)
		{
			if (layer.m_Name == name)
				return layer;
		}
		XYZ_ASSERT(false, "No layer with name {0}", name);
		return m_Layers.at(0);
	}
	const PhysicsWorld2D::Layer& PhysicsWorld2D::GetLayer(uint32_t index) const
	{
		return m_Layers.at(index);
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