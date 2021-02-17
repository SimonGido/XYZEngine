#pragma once
#include "PhysicsBody.h"


namespace XYZ {

	struct Manifold
	{
		void Initialize(const glm::vec2& gravity, float dt);            // Precalculations for impulse solving
		void Solve();                 // Generate contact information
		void ApplyImpulse();          // Solve impulse and apply
		void PositionalCorrection();  // Naive correction of positional penetration


		PhysicsBody* A;
		PhysicsBody* B;
		

	//private:
		glm::vec2 Normal;
		float PenetrationDepth;

		glm::vec2 Contacts[2] = { glm::vec2(0.0f), glm::vec2(0.0f) };
		uint32_t  ContactCount;

		float Restitution = 0.0f;
		float DynamicFriction = 0.0f;
		float StaticFriction = 0.0f;
	};
}