#pragma once
#include "AssetManager.h"
#include "XYZ/Utils/DataStructures/ThreadQueue.h"

namespace XYZ {

	class AssetLifeManager
	{
	public:
		struct AssetTimer
		{
			Ref<Asset> Asset;
			float	   TimeLeft;
		};

		static void Start(float aliveSeconds);
		static void Stop();

	private:
		static void threadFunc();

	private:
		static std::vector<AssetTimer> s_AssetTimers;
		static ThreadQueue<Ref<Asset>> s_AssetQueue;

		static float				   s_TimeAlive;
		static bool					   s_Running;

		static std::unique_ptr<std::thread>   s_UpdateThread;
	};

}