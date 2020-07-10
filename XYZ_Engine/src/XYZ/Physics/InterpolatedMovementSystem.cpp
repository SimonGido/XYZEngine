#include "stdafx.h"
#include "InterpolatedMovementSystem.h"




namespace XYZ {
	InterpolatedMovementSystem::InterpolatedMovementSystem()
	{
		m_Signature.set(XYZ::ECSManager::GetComponentType<InterpolatedMovement>());
		m_Signature.set(XYZ::ECSManager::GetComponentType<Transform2D>());
		m_Signature.set(XYZ::ECSManager::GetComponentType<GridBody>());
	}
	void InterpolatedMovementSystem::Update(float dt)
	{
		for (auto& it : m_Components)
		{
			if ((it.ActiveComponent->ActiveComponents & m_Signature) == m_Signature)
			{
				if (it.Interpolated->InProgress)
				{
					float x = it.Interpolated->Velocity.x * dt;
					float y = it.Interpolated->Velocity.y * dt;

					if (it.Interpolated->Distance.x > 0.0f)
					{
						it.Interpolated->Distance.x -= fabs(x);
						it.Transform->Translate(glm::vec3(x, 0, 0));
					}
					if (it.Interpolated->Distance.y > 0.0f)
					{
						it.Interpolated->Distance.y -= fabs(y);
						it.Transform->Translate(glm::vec3(0, y, 0));
					}

					if (it.Interpolated->Distance.x <= 0.0f
						&& it.Interpolated->Distance.y <= 0.0f)
					{
						it.Interpolated->InProgress = false;
						it.Interpolated->Velocity = glm::vec2(0);

						it.Interpolated->Distance.x = 0;
						it.Interpolated->Distance.y = 0;						
					}
				}
				else if (it.GridBody->NextCol != 0
					  || it.GridBody->NextRow != 0)
				{
					it.Interpolated->Distance.x = (float)fabs(it.GridBody->NextCol);
					it.Interpolated->Distance.y = (float)fabs(it.GridBody->NextRow);
					it.Interpolated->InProgress = true;
				}
				
			}
		}
	}
	void InterpolatedMovementSystem::Add(uint32_t entity)
	{
		Component component;
		component.ActiveComponent = ECSManager::GetComponent<ActiveComponent>(entity);
		component.GridBody = ECSManager::GetComponent<GridBody>(entity);
		component.Transform = ECSManager::GetComponent<Transform2D>(entity);
		component.Interpolated = ECSManager::GetComponent<InterpolatedMovement>(entity);
		component.Ent = entity;

		m_Components.push_back(component);
		XYZ_LOG_INFO("Entity with ID ", entity, " added");
	}
	void InterpolatedMovementSystem::Remove(uint32_t entity)
	{
		auto it = std::find(m_Components.begin(), m_Components.end(), entity);
		if (it != m_Components.end())
		{
			XYZ_LOG_INFO("Entity with id ", entity, " removed");
			*it = std::move(m_Components.back());
			m_Components.pop_back();
		}
	}
	bool InterpolatedMovementSystem::Contains(uint32_t entity)
	{
		auto it = std::find(m_Components.begin(), m_Components.end(), entity);
		if (it != m_Components.end())
			return true;
		return false;
	}
}