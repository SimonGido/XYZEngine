#pragma once
#include "Asset.h"

#include "XYZ/Utils/DataStructures/ThreadQueue.h"

namespace XYZ {

	class AssetLifeManager : public std::enable_shared_from_this<AssetLifeManager>
	{
	public:
		struct AssetTimer
		{
			Ref<Asset> Asset;
			float	   TimeLeft;
		};

		void Start(float aliveSeconds);
		void Stop();
		void PushAsset(Ref<Asset> asset);

	private:
		static void threadFunc(std::shared_ptr<AssetLifeManager> lifeManager);

	private:
		std::vector<AssetTimer> m_AssetTimers;
		ThreadQueue<Ref<Asset>> m_AssetQueue;

		float				    m_TimeAlive;
		bool					m_Running;

		std::unique_ptr<std::thread> m_UpdateThread;
	};

}