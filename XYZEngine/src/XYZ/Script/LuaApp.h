#pragma once

#include "XYZ/Scene/Scene.h"



struct lua_State;
namespace XYZ {

	class LuaApp
	{
	public:
		LuaApp(const std::string& filepath);

		void OnUpdate(Timestep ts);
		
		
		static void SetScene(Ref<Scene> scene);
	private:
		lua_State* m_L;
	};
}