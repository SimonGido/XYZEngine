#include "stdafx.h"
#include "AssetLifeManager.h"

#include "XYZ/Debug/Timer.h"

namespace XYZ {


	void AssetLifeManager::Start(float aliveSeconds)
	{
		if (m_Running)
			Stop();

		m_TimeAlive = aliveSeconds;
		m_Running = true;
		m_UpdateThread = std::make_unique<std::thread>(&AssetLifeManager::threadFunc, this->shared_from_this());
	}
	void AssetLifeManager::Stop()
	{
		m_Running = false;
		m_UpdateThread->join();
		m_UpdateThread.reset();
		m_AssetTimers.clear();
		m_AssetQueue.Clear();
	}
	void AssetLifeManager::PushAsset(Ref<Asset> asset)
	{
		m_AssetQueue.PushBack(asset);
	}
	void AssetLifeManager::threadFunc(std::shared_ptr<AssetLifeManager> lifeManager)
	{	
		float elapsed = 0.0f;
		while (lifeManager->m_Running)
		{
			Stopwatch timer;

			while (!lifeManager->m_AssetQueue.Empty())
			{
				lifeManager->m_AssetTimers.push_back({ lifeManager->m_AssetQueue.PopBack(), lifeManager->m_TimeAlive });
			}

			for (auto it = lifeManager->m_AssetTimers.begin(); it != lifeManager->m_AssetTimers.end(); )
			{
				it->TimeLeft -= elapsed;
				if (it->TimeLeft < 0.0f)
				{
					XYZ_CORE_INFO("Erasing asset reference");
					it = lifeManager->m_AssetTimers.erase(it);
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