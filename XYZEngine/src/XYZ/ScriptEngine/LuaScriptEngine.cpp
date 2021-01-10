#include "stdafx.h"
#include "LuaScriptEngine.h"


namespace XYZ {
	LuaTable::LuaTable(
		const std::string& tableName, 
		const std::initializer_list<LuaTableFunction>& functions,
		const std::initializer_list<LuaTableVariable>& variables
	)
		: m_TableName(tableName), m_Functions(functions), m_Variables(variables)
	{
	}

	void LuaScriptEngine::CreateTable(lua_State* state, const LuaTable& table)
	{
		std::string metaTable = table.GetName() + "MetaTable";
		luaL_newmetatable(state, metaTable.c_str());
		
		lua_newtable(state);
		int tableIndex = lua_gettop(state);
		lua_pushvalue(state, tableIndex);
		lua_setglobal(state, table.GetName().c_str());

		for (auto& it : table.GetVariables())
		{
			if (std::holds_alternative<float>(it.Variable))
			{
				lua_pushstring(state, it.Name.c_str());
				lua_pushnumber(state, std::get<float>(it.Variable));
				lua_settable(state, -3); // it pops key and number from stack, so next iteration it is again -3;
			}
			else if (std::holds_alternative<int>(it.Variable))
			{
				lua_pushstring(state, it.Name.c_str());
				lua_pushinteger(state, std::get<int>(it.Variable));
				lua_settable(state, -3);
			}
			else if (std::holds_alternative<bool>(it.Variable))
			{
				lua_pushstring(state, it.Name.c_str());
				lua_pushboolean(state, std::get<bool>(it.Variable));
				lua_settable(state, -3);
			}
			else if (std::holds_alternative<std::string>(it.Variable))
			{
				lua_pushstring(state, it.Name.c_str());
				lua_pushstring(state, std::get<std::string>(it.Variable).c_str());
				lua_settable(state, -3);
			}
		}

		for (auto& it : table.GetFunctions())
		{
			lua_pushcfunction(state, it.Function);
			lua_setfield(state, -2, it.Name.c_str());
		}	
	}
}