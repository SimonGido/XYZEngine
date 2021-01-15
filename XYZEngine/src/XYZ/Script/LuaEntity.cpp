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
		entity.m_Entity = { s_Scene->GetECS().FindEntity<SceneTagComponent>({ name }), &s_Scene->GetECS() };
		return entity;
	}


	void LuaEntity::SetScene(Ref<Scene> scene)
	{
		s_Scene = scene;
	}
}