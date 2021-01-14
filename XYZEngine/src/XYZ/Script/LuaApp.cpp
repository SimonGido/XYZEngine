#include "stdafx.h"
#include "LuaApp.h"

#include "XYZ/ECS/Entity.h"
#include "XYZ/Scene/Components.h"
#include "LuaEntity.h"

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}


#include <LuaBridge/LuaBridge.h>

namespace XYZ {
	static bool CheckLua(lua_State* L, int err)
	{
		if (err != 0)
		{
			XYZ_LOG_ERR(lua_tostring(L, -1));
			return false;
		}
		return true;
	}

	LuaApp::LuaApp(const std::string& filepath)
	{
		m_L = luaL_newstate();
		luaL_openlibs(m_L);

		luabridge::getGlobalNamespace(m_L)
			.beginClass <glm::vec3>("Vec3")
			.addProperty("x", &glm::vec3::x)
			.addProperty("y", &glm::vec3::y)
			.addProperty("z", &glm::vec3::z)
			.endClass();


		luabridge::getGlobalNamespace(m_L)
			.beginClass<TransformComponent>("TransformComponent")
			.addProperty("Translation", &TransformComponent::Translation)
			.addProperty("Rotation", &TransformComponent::Rotation)
			.addProperty("Scale", &TransformComponent::Scale)
			.endClass();
		

		luabridge::getGlobalNamespace(m_L)
			.beginClass<LuaEntity>("Entity")
			.addFunction("GetTransform", &LuaEntity::GetTransformComponent)
			.addStaticFunction("FindEntity", &LuaEntity::FindEntity)
			.addStaticFunction("CreateEntity", &LuaEntity::CreateEntity)
			.endClass();

		if (CheckLua(m_L, luaL_dofile(m_L, filepath.c_str())))
		{
		}		
	}

	void LuaApp::OnUpdate(Timestep ts)
	{
		luabridge::LuaRef handler = luabridge::getGlobal(m_L, "OnUpdate");
		handler((lua_Number)ts.GetSeconds());
	}

}