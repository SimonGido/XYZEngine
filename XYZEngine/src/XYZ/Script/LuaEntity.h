#pragma once
#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/Components.h"

namespace XYZ {


	class LuaEntity
	{
	public:
		LuaEntity();

		template <typename T>
		T& GetComponent()
		{
			return m_Entity.GetComponent<T>();
		}


		static LuaEntity CreateEntity(const std::string& name);
		static LuaEntity FindEntity(const std::string& name);

		static void SetScene(Ref<Scene> scene);
	private:
		Entity m_Entity;

		static Ref<Scene> s_Scene;
	};
}