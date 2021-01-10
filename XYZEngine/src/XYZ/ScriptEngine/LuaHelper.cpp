#include "stdafx.h"
#include "LuaHelper.h"




namespace XYZ {
	void LuaHelper::CreateDestructor(const char* metaTable, lua_State* state, LuaCallback callback)
	{
		luaL_newmetatable(state, metaTable); // -3
		lua_pushstring(state, "__gc");		 // -2
		lua_pushcfunction(state, callback);	 // -1
		lua_settable(state, -3);
	}
}