#pragma once
#include "XYZ/Scene/Scene.h"

struct lua_State;

namespace XYZ {

	class LuaScriptAPI
	{
	public:
		static void SetActiveScene(Ref<Scene> scene);

		static void LoadScript(lua_State ** state, const std::string& filepath);
		static void UnLoadScript(lua_State* state);

		static int SetTranslation(lua_State* state);
		static int GetTranslation(lua_State* state);
		static int GetEntity(lua_State* state);


		static int Vec2(lua_State* state);
		static int Vec3(lua_State* state);
		static int Vec4(lua_State* state);
	};
}