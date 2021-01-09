#include "stdafx.h"
#include "LuaScript.h"

#include "LuaScriptAPI.h"

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
		LuaScriptAPI::LoadScript(&m_LuaState, filepath);
	}

	LuaScript::~LuaScript()
	{
		LuaScriptAPI::UnLoadScript(m_LuaState);
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