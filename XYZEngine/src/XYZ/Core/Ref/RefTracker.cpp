#include "stdafx.h"
#include "RefTracker.h"

#include <unordered_set>

namespace XYZ {
	static std::unordered_set<void*> s_LiveReferences;
	static std::mutex				 s_LiveReferenceMutex;

	void RefTracker::addToLiveReferences(void* instance)
	{
		std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
		XYZ_ASSERT(instance, "");
		s_LiveReferences.insert(instance);
	}
	void RefTracker::removeFromLiveReferences(void* instance)
	{
		std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
		XYZ_ASSERT(instance, "");
		XYZ_ASSERT(s_LiveReferences.find(instance) != s_LiveReferences.end(), "");
		s_LiveReferences.erase(instance);
	}
	bool RefTracker::isLive(void* instance)
	{
		XYZ_ASSERT(instance, "");
		return s_LiveReferences.find(instance) != s_LiveReferences.end();
	}
}