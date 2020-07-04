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
		GuiSystem();
		virtual ~GuiSystem();

		virtual void Update(float dt);
		virtual void Add(Entity entity) override;
		virtual void Remove(Entity entity) override;
		virtual bool Contains(Entity entity) override;
		virtual void OnEvent(Event& event) override;

	private:
		void onMouseButtonPress(MouseButtonPressEvent& event);
		void onMouseButtonRelease(MouseButtonReleaseEvent& event);
		void onMouseMove(MouseMovedEvent& event);
		void onMouseScroll(MouseScrollEvent& event);

		bool collide(const glm::vec2& pos,const glm::vec2& size,const glm::vec2& point);
	private:
		struct Component : public System::Component
		{
			Transform2D* Transform;
			Widget* UI;
		};

		size_t m_Pressed = 0;

		std::vector<Component> m_Components;
		HashGrid2D<size_t> m_Grid;
	};
}