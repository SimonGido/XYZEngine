#include "stdafx.h"
#include "RendererQueueData.h"

#include "RendererAPI.h"
#include "Fence.h"

#include "XYZ/Debug/Profiler.h"


namespace XYZ {
	void RendererQueueData::Init(uint32_t framesInFlight)
	{
		m_FramesInFlight = framesInFlight;
		m_Pool.PushThread();

		if (RendererAPI::GetAPI() == RendererAPI::API::Vulkan)
		{
			// Initialize queues for creating and destroying resources for each frame
			m_ResourceQueues = new RenderCommandQueue[framesInFlight];
		}
	}
	void RendererQueueData::Shutdown()
	{
		BlockRenderThread();
		std::future<bool> result = m_Pool.PushJob<bool>([this] {
			m_RenderCommandQueue[0].Execute();
			m_RenderCommandQueue[1].Execute();
			return true;
		});
		result.wait();
		m_Pool.EraseThread(0);
		
		
		if (m_ResourceQueues != nullptr)
		{
			for (uint32_t i = 0; i < m_FramesInFlight; ++i)
				m_ResourceQueues[i].Execute();
			
			delete[]m_ResourceQueues;
			m_ResourceQueues = nullptr;
		}
	}

	void RendererQueueData::ExecuteRenderQueue()
	{
		RenderCommandQueue* queue = &m_RenderCommandQueue[m_RenderWriteIndex];
		#ifdef RENDER_THREAD_ENABLED

		m_RenderWriteIndex = m_RenderWriteIndex == 0 ? 1 : 0;
		m_RenderThreadFinished = m_Pool.PushJob<bool>([this, queue]() {
			XYZ_PROFILE_FUNC("RendererQueueData::ExecuteRenderQueue Job");
			queue->Execute();
			Fence::Create(UINT64_MAX);
			return true;
		});
		#else
			queue->Execute();
		#endif //  RENDER_THREAD_ENABLED	
	}
	void RendererQueueData::ExecuteResourceQueue(uint32_t frame)
	{
		XYZ_PROFILE_FUNC("RendererQueueData::ExecuteResourceQueue");
		RenderCommandQueue* queue = &m_ResourceQueues[frame];
		queue->Execute();
	}
	void RendererQueueData::BlockRenderThread()
	{
		#ifdef RENDER_THREAD_ENABLED
		XYZ_PROFILE_FUNC("RendererQueueData::BlockRenderThread");
		m_RenderThreadFinished.wait();
		#endif
	}

	RenderCommandQueue& RendererQueueData::GetRenderCommandQueue()
	{
		return m_RenderCommandQueue[m_RenderWriteIndex];
	}
	RenderCommandQueue& RendererQueueData::GetResourceCommandQueue(uint32_t frame)
	{
		return m_ResourceQueues[frame];
	}
}