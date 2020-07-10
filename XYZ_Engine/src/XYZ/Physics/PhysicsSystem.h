#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "PhysicsComponent.h"


namespace XYZ {
	/**
	* @class PhysicsSytem
	* @brief Represents system for handling physics
	*/
	class PhysicsSystem : public System
	{
	public:
		/**
		* Construct
		*/
		PhysicsSystem();
		virtual ~PhysicsSystem();

		virtual void Update(float dt);
		virtual void Add(uint32_t entity) override;
		virtual void Remove(uint32_t entity) override;
		virtual bool Contains(uint32_t entity) override;
	
	private:
		struct Component : public System::Component
		{
			RigidBody2D* RigidBody;
			InterpolatedMovement* Interpolated;
		};


	private:
		std::vector<Component> m_Components;

	};

}