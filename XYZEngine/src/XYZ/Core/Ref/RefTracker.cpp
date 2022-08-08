#include "stdafx.h"
#include "RefTracker.h"

#include <unordered_set>
#include <shared_mutex>

namespace XYZ {
	static std::unordered_set<void*> s_LiveReferences;
	static std::shared_mutex		 s_LiveReferenceMutex;

	void RefTracker::addToLiveReferences(void* instance)
	{
		std::unique_lock lock(s_LiveReferenceMutex);
		XYZ_ASSERT(instance, "");
		s_LiveReferences.insert(instance);
	}
	void RefTracker::removeFromLiveReferences(void* instance)
	{
		std::unique_lock lock(s_LiveReferenceMutex);
		XYZ_ASSERT(instance, "");
		XYZ_ASSERT(s_LiveReferences.find(instance) != s_LiveReferences.end(), "");
		s_LiveReferences.erase(instance);
	}
	bool RefTracker::isLive(void* instance)
	{
		XYZ_ASSERT(instance, "");
		std::shared_lock lock(s_LiveReferenceMutex);
		return s_LiveReferences.find(instance) != s_LiveReferences.end();
	}
}