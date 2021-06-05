#include "stdafx.h"
#include "ContactListener.h"
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {
	void ContactListener::BeginContact(b2Contact* contact)
	{
		SceneEntity* A = reinterpret_cast<SceneEntity*>(contact->GetFixtureA()->GetBody()->GetUserData().pointer);
		SceneEntity* B = reinterpret_cast<SceneEntity*>(contact->GetFixtureB()->GetBody()->GetUserData().pointer);
	}
	void ContactListener::EndContact(b2Contact* contact)
	{
	}
	void ContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
	{
		B2_NOT_USED(contact);
		B2_NOT_USED(oldManifold);
	}
	void ContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
	{
		B2_NOT_USED(contact);
		B2_NOT_USED(impulse);
	}
}