#pragma once


namespace XYZ {
	class RefTracker
	{
	private:
		static void addToLiveReferences(void* instance);
		static void removeFromLiveReferences(void* instance);
		static bool isLive(void* instance);

		template <typename T, typename Allocator>
		friend class Ref;

		template <typename T>
		friend class WeakRef;
	};
}