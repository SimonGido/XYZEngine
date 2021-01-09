#include "stdafx.h"
#include "LuaScriptAPI.h"

#include "XYZ/ECS/Entity.h"
#include "XYZ/Scene/Components.h"

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace XYZ {
	
	

	namespace Util {
		static int Add(lua_State* state)
		{
			XYZ_ASSERT(lua_istable(state, -2), "Left side of an operator add is not a table");
			XYZ_ASSERT(lua_istable(state, -1), "Right side of an operator add is not a table");

			lua_pushstring(state, "x");
			lua_gettable(state, -3); // Get left table
			lua_Number xLeft = lua_tonumber(state, -1);
			lua_pop(state, 1);

			lua_pushstring(state, "x");
			lua_gettable(state, -2); // Get right rable
			lua_Number xRight = lua_tonumber(state, -1);
			lua_pop(state, 1);

			lua_Number xRes = xLeft + xRight;
			

			lua_pushstring(state, "y");
			lua_gettable(state, -3); // Get left table
			lua_Number yLeft = lua_tonumber(state, -1);
			lua_pop(state, 1);

			lua_pushstring(state, "y");
			lua_gettable(state, -2); // Get right rable
			lua_Number yRight = lua_tonumber(state, -1);
			lua_pop(state, 1);

			lua_Number yRes = yLeft + yRight;

			LuaScriptAPI::Vec2(state);
			
			lua_pushstring(state, "x");
			lua_pushnumber(state, xRes);
			lua_rawset(state, -3);
			
			lua_pushstring(state, "y");
			lua_pushnumber(state, yRes);
			lua_rawset(state, -3);

			return 1;
		}

		static bool CheckLua(lua_State* L, int err)
		{
			if (err != LUA_OK)
			{
				XYZ_LOG_ERR(lua_tostring(L, -1));
				return false;
			}
			return true;
		}

	}


	static Ref<Scene> s_Scene = nullptr;

	
	void LuaScriptAPI::SetActiveScene(Ref<Scene> scene)
	{
		s_Scene = scene;
	}

	void LuaScriptAPI::LoadScript(lua_State** state, const std::string& filepath)
	{
		*state = luaL_newstate();
		luaL_openlibs(*state);

		lua_register(*state, "SetTranslation", LuaScriptAPI::SetTranslation);
		lua_register(*state, "GetTranslation", LuaScriptAPI::GetTranslation);
		lua_register(*state, "GetEntity", LuaScriptAPI::GetEntity);
					 
		lua_register(*state, "Vec2", LuaScriptAPI::Vec2);
		lua_register(*state, "Vec3", LuaScriptAPI::Vec3);
		lua_register(*state, "Vec4", LuaScriptAPI::Vec4);

		luaL_newmetatable(*state, "VectorMetaTable");
		lua_pushstring(*state, "__add");
		lua_pushcfunction(*state, Util::Add);
		lua_settable(*state, -3);

		if (Util::CheckLua(*state, luaL_dofile(*state, filepath.c_str())))
		{
		}
	}

	void LuaScriptAPI::UnLoadScript(lua_State* state)
	{
		lua_close(state);
	}

	int LuaScriptAPI::SetTranslation(lua_State* state)
	{
		Entity entity = Entity(lua_tonumber(state, -4), &s_Scene->GetECS());
		float x = lua_tonumber(state, -3);
		float y = lua_tonumber(state, -2);
		float z = lua_tonumber(state, -1);

		auto& transform = entity.GetStorageComponent<TransformComponent>();
		transform.Translation.x = x;
		transform.Translation.y = y;
		transform.Translation.z = z;

		return 0;
	}
	int LuaScriptAPI::GetTranslation(lua_State* state)
	{
		Entity entity = Entity(lua_tonumber(state, -1), &s_Scene->GetECS());

		auto& transform = entity.GetStorageComponent<TransformComponent>();

		lua_newtable(state);
		lua_pushstring(state, "x");
		lua_pushnumber(state, transform.Translation.x);
		lua_settable(state, -3);

		lua_pushstring(state, "y");
		lua_pushnumber(state, transform.Translation.y);
		lua_settable(state, -3);

		luaL_getmetatable(state, "VectorMetaTable");
		lua_setmetatable(state, -2);

		return 1;
	}

	int LuaScriptAPI::GetEntity(lua_State* state)
	{
		const char* name = lua_tostring(state, -1);
		uint32_t entity = s_Scene->GetECS().FindEntity<SceneTagComponent>(SceneTagComponent(name));
		lua_pushnumber(state, entity);
		return 1;
	}

	int LuaScriptAPI::Vec2(lua_State* state)
	{
		lua_newtable(state);
		lua_pushstring(state, "x");
		lua_pushnumber(state, 0);
		lua_settable(state,-3);

		lua_pushstring(state, "y");
		lua_pushnumber(state, 0);
		lua_settable(state, -3);

		luaL_getmetatable(state, "VectorMetaTable");
		lua_setmetatable(state, -2);

		return 1;
	}

	int LuaScriptAPI::Vec3(lua_State* state)
	{
		glm::vec3* vec = (glm::vec3*)lua_newuserdata(state, sizeof(glm::vec3));
		vec->x = 0.0f;
		vec->y = 0.0f;
		vec->z = 0.0f;
		return 1;
	}

	int LuaScriptAPI::Vec4(lua_State* state)
	{
		glm::vec4* vec = (glm::vec4*)lua_newuserdata(state, sizeof(glm::vec4));
		vec->x = 0.0f;
		vec->y = 0.0f;
		vec->z = 0.0f;
		vec->w = 0.0f;
		return 1;
	}

}