#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Utils/DataStructures/HashGrid2D.h"
#include "XYZ/Utils/DataStructures/Tree.h"
#include "XYZ/Physics/Transform.h"
#include "XYZ/Event/InputEvent.h"

#include "Widget.h"
#include "Canvas.h"

namespace XYZ {

	class GuiSystem : public System
	{
	public:
		GuiSystem(ECSManager* ecs);
		virtual ~GuiSystem();

		virtual void Update(float dt);
		virtual void Add(uint32_t entity) override;
		virtual void Remove(uint32_t entity) override;
		virtual bool Contains(uint32_t entity) override;

		bool OnMouseButtonPress(const glm::vec2& mousePos);
		bool OnMouseButtonRelease(const glm::vec2& mousePos);
		bool OnMouseMove(const glm::vec2& mousePos);
		bool OnMouseScroll(const glm::vec2& mousePos);
	private:
		bool collide(const glm::vec2& pos,const glm::vec2& size,const glm::vec2& point);
	private:
		ECSManager* m_ECS;
		struct Component : public System::Component
		{
			Transform* Transform;
			Widget* UI;
		};

		Widget* m_LastPressed = nullptr;
		Widget* m_LastHoovered = nullptr;
		std::vector<Component> m_Components;
		HashGrid2D<size_t> m_Grid;
	};
}