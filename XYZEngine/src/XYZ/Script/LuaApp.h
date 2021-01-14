#pragma once

#include "XYZ/Core/Timestep.h"

struct lua_State;
namespace XYZ {

	class LuaApp
	{
	public:
		LuaApp(const std::string& filepath);

		void OnUpdate(Timestep ts);
		
		
	private:
		lua_State* m_L;
	};
}