#pragma once
#include "Script.h"
#include "XYZ/ECS/Entity.h"


struct lua_State;
namespace XYZ {

	class LuaScript : public Script
	{
	public:
		LuaScript(const std::string& filepath);
		virtual ~LuaScript() override;

		virtual void OnCreate() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnDestroy() override;

	private:

		lua_State* m_LuaState;
	};
}