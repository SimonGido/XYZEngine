#include "stdafx.h"
#include "AssetLifeManager.h"


namespace XYZ {

	std::vector<AssetLifeManager::AssetTimer> AssetLifeManager::s_AssetTimers;
	ThreadQueue<Ref<Asset>>					  AssetLifeManager::s_AssetQueue;
	float									  AssetLifeManager::s_TimeAlive = 0.0f;
	bool									  AssetLifeManager::s_Running = false;
	std::unique_ptr<std::thread>			  AssetLifeManager::s_UpdateThread;

	void AssetLifeManager::Start(float aliveSeconds)
	{
		s_TimeAlive = aliveSeconds;
		s_Running = true;
		s_UpdateThread = std::make_unique<std::thread>(&AssetLifeManager::threadFunc);
		
		AssetManager::s_OnAssetLoaded = [](Ref<Asset> asset) {
			s_AssetQueue.PushBack(asset);
		};
	}
	void AssetLifeManager::Stop()
	{
		s_Running = false;
		s_UpdateThread->join();
		s_UpdateThread.reset();
		s_AssetTimers.clear();
		s_AssetQueue.Clear();
	}
	void AssetLifeManager::threadFunc()
	{	
		float elapsed = 0.0f;
		while (s_Running)
		{
			Stopwatch timer;

			while (!s_AssetQueue.Empty())
			{
				s_AssetTimers.push_back({ s_AssetQueue.PopBack(), s_TimeAlive });
			}

			for (auto it = s_AssetTimers.begin(); it != s_AssetTimers.end(); )
			{
				it->TimeLeft -= elapsed;
				if (it->TimeLeft < 0.0f)
				{
					XYZ_CORE_INFO("Erasing asset reference");
					it = s_AssetTimers.erase(it);
				}
				else
				{
					it++;
				}
			}
			elapsed = timer.ElapsedSeconds();
		}
	}

}