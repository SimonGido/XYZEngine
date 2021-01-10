#pragma once

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <variant>

namespace XYZ {
	typedef int (*LuaFunction)(lua_State*);

	struct LuaTableVariable
	{
		std::string Name;
		std::variant<bool, int, float, std::string> Variable;
	};

	struct LuaTableFunction
	{
		std::string Name;
		LuaFunction Function;
	};

	class LuaTable
	{
	public:
		LuaTable(
			const std::string& tableName, 
			const std::initializer_list<LuaTableFunction>& functions,
			const std::initializer_list<LuaTableVariable>& variables
		);

		inline const std::string& GetName() const { return m_TableName; }
		inline const std::vector<LuaTableFunction>& GetFunctions() const { return m_Functions; }
		inline const std::vector<LuaTableVariable>& GetVariables() const { return m_Variables; }
	private:
		std::string m_TableName;
		std::vector<LuaTableFunction> m_Functions;
		std::vector<LuaTableVariable> m_Variables;

	};


	class LuaScriptEngine
	{
	public:
		static void CreateTable(lua_State* state, const LuaTable& table);

		
	};


}