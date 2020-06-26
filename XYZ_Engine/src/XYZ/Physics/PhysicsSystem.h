#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "PhysicsComponent.h"

#include "XYZ/Renderer/Renderable2D.h"


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
		~PhysicsSystem();

		virtual void Update(float dt);
		virtual void Add(Entity entity) override;
		virtual void Remove(Entity entity) override;
		virtual bool Contains(Entity entity) override;
	
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