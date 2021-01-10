#include "stdafx.h"
#include "LuaScript.h"

#include "LuaScriptAPI.h"
#include "LuaScriptEngine.h"

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace XYZ {
	static bool CheckLua(lua_State* L, int err)
	{
		if (err != LUA_OK)
		{
			XYZ_LOG_ERR(lua_tostring(L, -1));
			return false;
		}
		return true;
	}


	LuaScript::LuaScript(const std::string& filepath)
	{
		m_LuaState = luaL_newstate();
		luaL_openlibs(m_LuaState);

		LuaScriptEngine::CreateTable(m_LuaState, LuaTable(
			"Entity", 
			{
				{"GetTranslation", LuaScriptAPI::GetTranslation},
				{"SetTranslation", LuaScriptAPI::SetTranslation},
			    {"Find",		   LuaScriptAPI::Find}
			},
			{}
		));
		LuaScriptEngine::CreateTable(m_LuaState, LuaTable(
			"Math", 
			{
				{"Vec2", LuaScriptAPI::Vec2},
				{"Vec3", LuaScriptAPI::Vec3},
				{"Vec4", LuaScriptAPI::Vec4}
			},
			{}

		));
		LuaScriptEngine::CreateTable(m_LuaState, LuaTable(
			"Input",
			{
				{"IsKeyPressed", LuaScriptAPI::IsKeyPressed},
			},
			{
				{"KeyW", ToUnderlying(KeyCode::XYZ_KEY_W) },
				{"KeyA", ToUnderlying(KeyCode::XYZ_KEY_A) },
				{"KeyS", ToUnderlying(KeyCode::XYZ_KEY_S) },
				{"KeyD", ToUnderlying(KeyCode::XYZ_KEY_D) }
			}

		));


		if (CheckLua(m_LuaState, luaL_dofile(m_LuaState, filepath.c_str())))
		{
		}
	}

	LuaScript::~LuaScript()
	{
		lua_close(m_LuaState);
	}


	void LuaScript::OnCreate()
	{
		constexpr int numArgs = 0;
		constexpr int returnVals = 0;

		lua_getglobal(m_LuaState, "OnCreate");
		if (lua_isfunction(m_LuaState, -1))
		{
			int err = 0;
			lua_pcall(m_LuaState, numArgs, returnVals, err);
			if (CheckLua(m_LuaState, err))
			{
			}
		}
	}
	void LuaScript::OnUpdate(Timestep ts)
	{
		constexpr int numArgs = 1;
		constexpr int returnVals = 0;

		lua_getglobal(m_LuaState, "OnUpdate");
		if (lua_isfunction(m_LuaState, -1))
		{
			lua_pushnumber(m_LuaState, ts);
			int err = 0;
			lua_pcall(m_LuaState, numArgs, returnVals, err);
			if (CheckLua(m_LuaState, err))
			{
			}
		}
	}
	void LuaScript::OnDestroy()
	{
	}
}