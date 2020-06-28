#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Utils/DataStructures/Tree.h"
#include "XYZ/Physics/Transform.h"
#include "Canvas.h"
#include "UIComponent.h"

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

	private:
		struct Component : public System::Component
		{
			Transform2D* Transform;
			UIComponent* UI;
		};

		Tree<Component> m_Widgets;
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