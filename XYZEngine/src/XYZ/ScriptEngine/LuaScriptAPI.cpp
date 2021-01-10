#include "stdafx.h"
#include "LuaScriptAPI.h"

#include "XYZ/Core/Input.h"
#include "XYZ/ECS/Entity.h"
#include "XYZ/Scene/Components.h"



extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace XYZ {
	
	static Ref<Scene> s_Scene = nullptr;

	
	void LuaScriptAPI::SetActiveScene(Ref<Scene> scene)
	{
		s_Scene = scene;
	}
}

namespace XYZ {

	template <>
	void LuaHelper::PushToTable<float>(lua_State* state, const char* key, float val)
	{
		lua_pushstring(state, key);
		lua_pushnumber(state, val);
		lua_settable(state, -3);
	}
	template <>
	void LuaHelper::PushToTable<int>(lua_State* state, const char* key, int val)
	{
		lua_pushstring(state, key);
		lua_pushinteger(state, val);
		lua_settable(state, -3);
	}
	template <>
	void LuaHelper::PushToTable<bool>(lua_State* state, const char* key, bool val)
	{
		lua_pushstring(state, key);
		lua_pushboolean(state, val);
		lua_settable(state, -3);
	}
	template <>
	void LuaHelper::PushToTable<const char*>(lua_State* state, const char* key, const char* val)
	{
		lua_pushstring(state, key);
		lua_pushstring(state, val);
		lua_settable(state, -3);
	}
	
	int LuaScriptAPI::GetTranslation(lua_State* state)
	{
		XYZ::Entity entity = XYZ::Entity(lua_tonumber(state, -1), &s_Scene->GetECS());

		auto& transform = entity.GetStorageComponent<TransformComponent>();

		lua_newtable(state);
		LuaHelper::PushToTable<float>(state, "x", transform.Translation.x);
		LuaHelper::PushToTable<float>(state, "y", transform.Translation.y);
		LuaHelper::PushToTable<float>(state, "z", transform.Translation.z);

		return 1;
	}

	int LuaScriptAPI::SetTranslation(lua_State* state)
	{
		XYZ::Entity entity = XYZ::Entity(lua_tonumber(state, -4), &s_Scene->GetECS());
		float x = lua_tonumber(state, -3);
		float y = lua_tonumber(state, -2);
		float z = lua_tonumber(state, -1);
	
		auto& transform = entity.GetStorageComponent<TransformComponent>();
		transform.Translation.x = x;
		transform.Translation.y = y;
		transform.Translation.z = z;
	
		return 0;
	}

	int LuaScriptAPI::Find(lua_State* state)
	{
		const char* name = lua_tostring(state, -1);
		uint32_t entity = s_Scene->GetECS().FindEntity<SceneTagComponent>(SceneTagComponent(name));
		lua_pushnumber(state, entity);
		return 1;
	}

	int LuaScriptAPI::Vec2(lua_State* state)
	{
		lua_newtable(state);
		LuaHelper::PushToTable<float>(state, "x", 0.0f);
		LuaHelper::PushToTable<float>(state, "y", 0.0f);
		return 1;
	}

	int LuaScriptAPI::Vec3(lua_State* state)
	{
		lua_newtable(state);
		LuaHelper::PushToTable<float>(state, "x", 0.0f);
		LuaHelper::PushToTable<float>(state, "y", 0.0f);
		LuaHelper::PushToTable<float>(state, "z", 0.0f);
		return 1;
	}

	int LuaScriptAPI::Vec4(lua_State* state)
	{
		lua_newtable(state);
		LuaHelper::PushToTable<float>(state, "x", 0.0f);
		LuaHelper::PushToTable<float>(state, "y", 0.0f);
		LuaHelper::PushToTable<float>(state, "z", 0.0f);
		LuaHelper::PushToTable<float>(state, "w", 0.0f);
		return 1;
	}
	int LuaScriptAPI::IsKeyPressed(lua_State* state)
	{
		int key = lua_tonumber(state, -1);
		KeyCode code = static_cast<KeyCode>(key);
		lua_pushboolean(state, Input::IsKeyPressed(code));
		
		return 1;
	}
}


