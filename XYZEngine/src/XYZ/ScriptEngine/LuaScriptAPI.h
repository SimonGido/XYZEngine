#pragma once
#include "XYZ/Scene/Scene.h"

struct lua_State;

namespace XYZ {

	class LuaHelper
	{
	public:
		template <typename T>
		static void PushToTable(lua_State* state, const char* key, T val);
	};

	class LuaScriptAPI
	{
	public:
		static void SetActiveScene(Ref<Scene> scene);

		// Entity
		static int GetTranslation(lua_State* state);
		static int SetTranslation(lua_State* state);
		static int Find(lua_State* state);

		// Math
		static int Vec2(lua_State* state);
		static int Vec3(lua_State* state);
		static int Vec4(lua_State* state);

		// Input
		static int IsKeyPressed(lua_State* state);
	};
}