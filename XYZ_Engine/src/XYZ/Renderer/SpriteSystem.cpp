#include "stdafx.h"
#include "SpriteSystem.h"


namespace XYZ {
	SpriteSystem::SpriteSystem()
	{
		m_Signature.set(ECSManager::GetComponentType<SpriteAnimation>());
		m_Signature.set(ECSManager::GetComponentType<SpriteRenderComponent>());
	}
	void SpriteSystem::Update(float dt)
	{
		for (auto it : m_Components)
		{
			it.Animation->Update(dt);
			it.Renderable->SetSubTexture(it.Animation->GetCurrentKeyFrame());
		}
	}
	void SpriteSystem::Add(uint32_t entity)
	{
		XYZ_LOG_INFO("Entity with ID ", entity, " added");
	
		Component component;
		component.Animation = ECSManager::GetComponent<SpriteAnimation>(entity);
		component.Renderable = ECSManager::GetComponent<SpriteRenderComponent>(entity);
		component.Ent = entity;

		m_Components.push_back(component);
	}
	void SpriteSystem::Remove(uint32_t entity)
	{
		auto it = std::find(m_Components.begin(), m_Components.end(), entity);
		if (it != m_Components.end())
		{
			XYZ_LOG_INFO("Entity with id ", entity, " removed");
			*it = std::move(m_Components.back());
			m_Components.pop_back();
		}
	}
	bool SpriteSystem::Contains(uint32_t entity)
	{
		auto it = std::find(m_Components.begin(), m_Components.end(), entity);
		if (it != m_Components.end())
			return true;
		return false;
	}
}