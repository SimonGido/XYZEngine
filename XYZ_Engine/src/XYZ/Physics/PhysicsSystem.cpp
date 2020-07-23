#include "stdafx.h"
#include "PhysicsSystem.h"


#include "XYZ/Timer.h"

namespace XYZ {
	PhysicsSystem::PhysicsSystem(ECSManager* ecs)
		:
		m_ECS(ecs)
	{
		m_Signature.set(m_ECS->GetComponentType<XYZ::RigidBody2D>());
		m_Signature.set(m_ECS->GetComponentType<XYZ::InterpolatedMovement>());
	}
	PhysicsSystem::~PhysicsSystem()
	{

	}
	void PhysicsSystem::Update(float dt)
	{
		for (auto& it : m_Components)
		{
			if ((it.ActiveComponent->ActiveComponents & m_Signature) == m_Signature)
			{
				
			}
		}
	}

	void PhysicsSystem::Add(uint32_t entity)
	{
		Component component;
		component.RigidBody = m_ECS->GetComponent<RigidBody2D>(entity);
		component.Interpolated = m_ECS->GetComponent<InterpolatedMovement>(entity);
		component.ActiveComponent = m_ECS->GetComponent<ActiveComponent>(entity);


		m_Components.push_back(component);
		XYZ_LOG_INFO("Entity with ID ", entity, " added");
	}

	void PhysicsSystem::Remove(uint32_t entity)
	{
		auto it = std::find(m_Components.begin(), m_Components.end(), entity);
		if (it != m_Components.end())
		{
			XYZ_LOG_INFO("Entity with id ", entity, " removed");

			*it = std::move(m_Components.back());
			m_Components.pop_back();
		}
	}

	bool PhysicsSystem::Contains(uint32_t entity)
	{
		auto it = std::find(m_Components.begin(), m_Components.end(), entity);
		return (it != m_Components.end());
	}

}