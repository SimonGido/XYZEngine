#pragma once


extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace XYZ {

	typedef int (*LuaCallback)(lua_State*);

	class LuaHelper
	{
	public:
		template <typename T>
		static int Constructor(const char* metaTable, lua_State* state)
		{
			void* pointer = lua_newuserdata(state, sizeof(T)); // -2
			new (pointer) T();
			luaL_getmetatable(state, metaTable); // -1
			XYZ_ASSERT(lua_istable(state, -1), "Metatable ", metaTable, " does not exist");
			lua_setmetatable(state, -2);
			return 1;
		}

		template <typename T>
		static int Destructor(lua_State* state)
		{
			T* pointer = (T*)lua_touserdata(state, -1);
			pointer->~T();
			return 0;
		}

		static void CreateDestructor(const char* metaTable, lua_State* state, LuaCallback callback);
	};

}