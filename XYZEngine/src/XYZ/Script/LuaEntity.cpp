#include "stdafx.h"
#include "LuaEntity.h"


namespace XYZ {
	Ref<Scene> LuaEntity::s_Scene = nullptr;

	LuaEntity::LuaEntity()
	{
	}


	LuaEntity LuaEntity::CreateEntity(const std::string& name)
	{
		LuaEntity entity;
		entity.m_Entity = s_Scene->CreateEntity(name, GUID());
		return entity;
	}

	LuaEntity LuaEntity::FindEntity(const std::string& name)
	{
		LuaEntity entity;
		entity.m_Entity = { s_Scene->m_ECS.FindEntity<SceneTagComponent>({ name }), s_Scene.Raw() };
		return entity;
	}


	void LuaEntity::SetActiveScene(Ref<Scene> scene)
	{
		s_Scene = scene;
	}
}