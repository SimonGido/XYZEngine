#include "stdafx.h"
#include "SpriteSystem.h"


namespace XYZ {

	static void FillTexCoords(const glm::vec4& texCoord, RenderComponent2D* renderable)
	{
		renderable->Mesh->Vertices[0].TexCoord = { texCoord.x, texCoord.y };
		renderable->Mesh->Vertices[1].TexCoord = { texCoord.z, texCoord.y };
		renderable->Mesh->Vertices[2].TexCoord = { texCoord.z, texCoord.w };
		renderable->Mesh->Vertices[3].TexCoord = { texCoord.x, texCoord.w };		 
	}

	SpriteSystem::SpriteSystem()
	{
		m_Signature.set(ECSManager::GetComponentType<SpriteAnimation>());
		m_Signature.set(ECSManager::GetComponentType<RenderComponent2D>());
	}
	void SpriteSystem::Update(float dt)
	{
		for (auto it : m_Components)
		{
			it.Animation->Update(dt);
			FillTexCoords(it.Animation->GetCurrentKeyFrame()->GetTexCoords(), it.Renderable);
		}
	}
	void SpriteSystem::Add(uint32_t entity)
	{
		XYZ_LOG_INFO("Entity with ID ", entity, " added");
	
		Component component;
		component.Animation = ECSManager::GetComponent<SpriteAnimation>(entity);
		component.Renderable = ECSManager::GetComponent<RenderComponent2D>(entity);
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