#include "stdafx.h"
#include "SceneEntity.h"


namespace XYZ {
	SceneEntity::SceneEntity()
		:
		m_Scene(nullptr)
	{
	}
	SceneEntity::SceneEntity(const SceneEntity& other)
		:
		m_Scene(other.m_Scene),
		m_ID(other.m_ID)
	{}
	SceneEntity::SceneEntity(entt::entity id, Scene * scene)
		:
		m_Scene(scene),
		m_ID(id)
	{
	}
	void SceneEntity::Destroy()
	{
		m_Scene->DestroyEntity(*this);
	}
	bool SceneEntity::IsValid() const
	{
		return m_Scene && GetRegistry()->valid(m_ID);
	}
	const entt::registry* SceneEntity::GetRegistry() const
	{
		if (m_Scene)
			return &m_Scene->m_Registry;
		return nullptr;
	}
	
	SceneEntity& SceneEntity::operator=(const SceneEntity& other)
	{
		m_Scene = other.m_Scene;
		m_ID = other.m_ID;
		return *this;
	}
	bool SceneEntity::operator==(const SceneEntity& other) const
	{
		return (m_ID == other.m_ID && m_Scene == other.m_Scene);
	}
}