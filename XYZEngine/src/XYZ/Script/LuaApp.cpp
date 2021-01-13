#include "stdafx.h"
#include "LuaApp.h"

#include "XYZ/ECS/Entity.h"
#include "XYZ/Scene/Components.h"


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
	
	static Ref<Scene> s_Scene = nullptr;

	class LuaEntity
	{
	public:
		LuaEntity(std::string name)
		{
			m_Entity = { s_Scene->GetECS().FindEntity<SceneTagComponent>(name), &s_Scene->GetECS() };
		}
		void SetTranslation(float x, float y, float z)
		{
			m_Entity.GetComponent<TransformComponent>().Translation = { x, y ,z };
		}

		float GetTranslationX() const
		{
			return m_Entity.GetComponent<TransformComponent>().Translation.x;
		}

		glm::vec3 GetTranslation() const
		{
			return m_Entity.GetComponent<TransformComponent>().Translation;
		}

	private:
		Entity m_Entity;
	};

	struct VecHelper
	{
		///////////////////////////////////
		static float getX(const glm::vec3* vec)
		{
			return vec->x;
		}

		static void setX(glm::vec3* vec, float x)
		{
			vec->x = x;
		}
		///////////////////////////////////
		static float getY(const glm::vec3* vec)
		{
			return vec->y;
		}

		static void setY(glm::vec3* vec, float y)
		{
			vec->y = y;
		}
		///////////////////////////////////
		static float getZ(const glm::vec3* vec)
		{
			return vec->z;
		}

		static void setZ(glm::vec3* vec, float z)
		{
			vec->z = z;
		}
	};

	LuaApp::LuaApp(const std::string& filepath)
	{
		m_L = luaL_newstate();
		luaL_openlibs(m_L);

		luabridge::getGlobalNamespace(m_L)
			.beginNamespace("test")
			.beginClass <glm::vec3>("Vec3")
			.addProperty("x", &VecHelper::getX, &VecHelper::setX)
			.addProperty("y", &VecHelper::getY, &VecHelper::setY)
			.addProperty("z", &VecHelper::getZ, &VecHelper::setZ)
			.endClass()
			.endNamespace();


		luabridge::getGlobalNamespace(m_L)
			.beginClass<LuaEntity>("LuaEntity")
			.addConstructor<void(*) (std::string)>()
			.addFunction("SetTranslation", &LuaEntity::SetTranslation)
			.addFunction("GetTranslationX", &LuaEntity::GetTranslationX)
			.addFunction("GetTranslation", &LuaEntity::GetTranslation)
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
	void LuaApp::SetScene(Ref<Scene> scene)
	{
		s_Scene = scene;
	}
}