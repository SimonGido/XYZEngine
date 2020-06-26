#include "stdafx.h"
#include "PhysicsSystem.h"


#include "XYZ/Timer.h"

namespace XYZ {
	PhysicsSystem::PhysicsSystem()
	{
		m_Signature.set(XYZ::ECSManager::Get().GetComponentType<XYZ::RigidBody2D>());
		m_Signature.set(XYZ::ECSManager::Get().GetComponentType<XYZ::InterpolatedMovement>());
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

	void PhysicsSystem::Add(Entity entity)
	{
		Component component;
		component.RigidBody = ECSManager::Get().GetComponent<RigidBody2D>(entity);
		component.Interpolated = ECSManager::Get().GetComponent<InterpolatedMovement>(entity);
		component.ActiveComponent = ECSManager::Get().GetComponent<ActiveComponent>(entity);


		m_Components.push_back(component);
		XYZ_LOG_INFO("Entity with ID ", entity, " added");
	}

	void PhysicsSystem::Remove(Entity entity)
	{
		auto it = std::find(m_Components.begin(), m_Components.end(), entity);
		if (it != m_Components.end())
		{
			XYZ_LOG_INFO("Entity with id ", entity, " removed");

			*it = std::move(m_Components.back());
			m_Components.pop_back();
		}
	}

	bool PhysicsSystem::Contains(Entity entity)
	{
		auto it = std::find(m_Components.begin(), m_Components.end(), entity);
		if (it != m_Components.end())
			return true;
		return false;
	}

}