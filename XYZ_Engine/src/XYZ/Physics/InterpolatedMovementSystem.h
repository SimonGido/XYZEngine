#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "PhysicsComponent.h"
#include "Transform.h"


namespace XYZ {
	/*! @class InterpolatedMovementSystem
	* @brief InterpolatedMovementystem takes care for interpolated movement of entities with interpolated movement component
	*/
	class InterpolatedMovementSystem : public System
	{
	public:
		InterpolatedMovementSystem();
		virtual void Update(float dt);
		virtual void Add(uint32_t entity) override;
		virtual void Remove(uint32_t entity) override;
		virtual bool Contains(uint32_t entity) override;

	private:
		struct Component : public System::Component
		{
			InterpolatedMovement* Interpolated;
			GridBody* GridBody;
			Transform2D* Transform;
		};

		std::vector<Component> m_Components;
	};

}