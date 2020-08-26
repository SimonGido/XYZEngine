#pragma once
#include "XYZ/ECS/Entity.h"


namespace XYZ {

	class EntityEditor
	{
	public:
		EntityEditor(Entity entity);

		void OnInGuiRender();
	private:
		void drawComponents();

	private:
		Entity m_Entity;
	};
}