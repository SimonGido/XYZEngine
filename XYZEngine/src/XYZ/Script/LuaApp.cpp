#include "stdafx.h"
#include "LuaApp.h"

#include "XYZ/ECS/Entity.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Core/Input.h"
#include "XYZ/Core/KeyCodes.h"
#include "LuaEntity.h"

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}


#include <LuaBridge/LuaBridge.h>

namespace luabridge {
	template <typename T>
	struct EnumWrapper 
	{
		static inline
			typename std::enable_if<std::is_enum<T>::value, void>::type
			push(lua_State* L, T value) 
		{
			lua_pushnumber(L, static_cast<std::size_t>(value));
		}

		static inline
			typename std::enable_if<std::is_enum<T>::value, T>::type
			get(lua_State* L, int index) 
		{
			return static_cast<T>(lua_tointeger(L, index));
		}
	};

	template <>
	struct luabridge::Stack<XYZ::KeyCode> : EnumWrapper<XYZ::KeyCode> 
	{
	};
}

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


		// Input
		luabridge::getGlobalNamespace(m_L)
			.beginClass <Input>("Input")
			.addStaticFunction("IsMouseButtonPressed", Input::IsMouseButtonPressed)
			.addStaticFunction("IsKeyPressed", Input::IsKeyPressed)
			.addStaticFunction("GetMouseX", Input::GetMouseX)
			.addStaticFunction("GetMouseY", Input::GetMouseY)
			.endClass();


		// Math
		luabridge::getGlobalNamespace(m_L)
			.beginClass <glm::vec2>("Vec2")
			.addProperty("x", &glm::vec2::x)
			.addProperty("y", &glm::vec2::y)
			.endClass();
		
		luabridge::getGlobalNamespace(m_L)
			.beginClass <glm::vec3>("Vec3")
			.addProperty("x", &glm::vec3::x)
			.addProperty("y", &glm::vec3::y)
			.addProperty("z", &glm::vec3::z)
			.endClass();

		luabridge::getGlobalNamespace(m_L)
			.beginClass <glm::vec4>("Vec4")
			.addProperty("x", &glm::vec4::x)
			.addProperty("y", &glm::vec4::y)
			.addProperty("z", &glm::vec4::z)
			.addProperty("w", &glm::vec4::w)
			.endClass();


		// Components
		luabridge::getGlobalNamespace(m_L)
			.beginClass<TransformComponent>("TransformComponent")
			.addProperty("Translation", &TransformComponent::Translation)
			.addProperty("Rotation", &TransformComponent::Rotation)
			.addProperty("Scale", &TransformComponent::Scale)
			.endClass();
		


		// Entity
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

	LuaApp::~LuaApp()
	{
		lua_close(m_L);
	}

	void LuaApp::OnUpdate(Timestep ts)
	{
		try
		{
			luabridge::LuaRef handler = luabridge::getGlobal(m_L, "OnUpdate");
			handler((lua_Number)ts.GetSeconds());
		}
		catch (const std::exception& e)
		{
			XYZ_LOG_ERR("Exception OnUpdate: ", e.what());
		}	
	}
}