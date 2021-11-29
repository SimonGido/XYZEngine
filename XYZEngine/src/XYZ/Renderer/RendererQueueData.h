#pragma once
#include "XYZ/Utils/DataStructures/ThreadPass.h"
#include "XYZ/Core/ThreadPool.h"

#include "RenderCommandQueue.h"


#include <shared_mutex>

namespace XYZ {


	class RendererQueueData
	{
	public:
		void Init(uint32_t framesInFlight);
		void Shutdown();

		void ExecuteRenderQueue();
		void ExecuteResourceQueue(uint32_t index);

		void BlockRenderThread();


		RenderCommandQueue&			   GetRenderCommandQueue();
		ScopedLock<RenderCommandQueue> GetResourceQueue(uint32_t index);

		ThreadPool& GetThreadPool() { return m_Pool; }
		bool RenderCommandQueuesEmpty() const;
	private:
		uint32_t		   m_FramesInFlight = 0;
		RenderCommandQueue m_RenderCommandQueue[2];
		
		struct ResourceCommandQueue
		{
			RenderCommandQueue  Queue;
			std::shared_mutex   Mutex;
		};

		ResourceCommandQueue* m_ResourceQueues;

		ThreadPool		   m_Pool;
		uint32_t		   m_RenderWriteIndex = 0;

		std::future<bool>  m_RenderThreadFinished;
	};

}