#include "stdafx.h"
#include "SpriteSystem.h"


namespace XYZ {
	SpriteSystem::SpriteSystem()
	{
		m_Signature.set(ECSManager::Get().GetComponentType<SpriteAnimation>());
		m_Signature.set(ECSManager::Get().GetComponentType<SpriteRenderComponent>());
	}
	void SpriteSystem::Update(float dt)
	{
		for (auto it : m_Components)
		{
			it.Animation->Update(dt);
			it.Renderable->SetSubTexture(it.Animation->GetCurrentKeyFrame());
		}
	}
	void SpriteSystem::Add(Entity entity)
	{
		XYZ_LOG_INFO("Entity with ID ", entity, " added");
	
		Component component;
		component.Animation = ECSManager::Get().GetComponent<SpriteAnimation>(entity);
		component.Renderable = ECSManager::Get().GetComponent<SpriteRenderComponent>(entity);
		component.Ent = entity;

		m_Components.push_back(component);
	}
	void SpriteSystem::Remove(Entity entity)
	{
		auto it = std::find(m_Components.begin(), m_Components.end(), entity);
		if (it != m_Components.end())
		{
			XYZ_LOG_INFO("Entity with id ", entity, " removed");
			*it = std::move(m_Components.back());
			m_Components.pop_back();
		}
	}
	bool SpriteSystem::Contains(Entity entity)
	{
		auto it = std::find(m_Components.begin(), m_Components.end(), entity);
		if (it != m_Components.end())
			return true;
		return false;
	}
}