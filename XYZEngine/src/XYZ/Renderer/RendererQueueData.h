#pragma once
#include "XYZ/Utils/DataStructures/ThreadPass.h"
#include "XYZ/Core/ThreadPool.h"

#include "RenderCommandQueue.h"


#include <shared_mutex>

namespace XYZ {



	enum RenderQueueType
	{
		Default,
		Overlay,
		NumTypes
	};

	class RendererQueueData
	{
	public:
		void Init(uint32_t framesInFlight);
		void Shutdown();

		void ExecuteRenderQueue();
		void ExecuteResourceQueue(uint32_t frame);
		void BlockRenderThread();


		RenderCommandQueue& GetRenderCommandQueue();
		RenderCommandQueue& GetResourceCommandQueue(uint32_t frame);

		ThreadPool& GetThreadPool() { return m_Pool; }

	private:
		uint32_t		   m_FramesInFlight = 0;
		RenderCommandQueue m_RenderCommandQueue[2];
		std::mutex		   m_RenderMutex;
		
		RenderCommandQueue* m_ResourceQueues;

		ThreadPool		   m_Pool;
		uint32_t		   m_RenderWriteIndex = 0;

		std::future<bool>  m_RenderThreadFinished;
	};

}