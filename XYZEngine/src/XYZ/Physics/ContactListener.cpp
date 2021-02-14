#include "stdafx.h"
#include "ContactListener.h"

namespace XYZ {
	void ContactListener::BeginContact(b2Contact* contact)
	{
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