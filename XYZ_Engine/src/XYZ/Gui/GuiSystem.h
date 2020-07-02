#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Utils/DataStructures/Tree.h"
#include "XYZ/Physics/Transform.h"
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
		struct Component : public System::Component
		{
			Transform2D* Transform;
			Widget* Widg;
		};

		std::vector<Component> m_Components;
	private:
		struct Setup
		{
			void operator ()(Component& parent, Component& child)
			{
				child.Transform->SetParent(parent.Transform);
			}
		};
	
		struct Propagate
		{
			void operator ()(Component& parent, Component& child)
			{
				child.Transform->CalculateWorldTransformation();
			}
		};

	};
}