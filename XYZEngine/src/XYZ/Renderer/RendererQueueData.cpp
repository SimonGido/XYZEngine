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
		m_ResourceQueues = new ResourceCommandQueue[framesInFlight];
	}
	void RendererQueueData::Shutdown()
	{
		BlockRenderThread();
		while (!RenderCommandQueuesEmpty())
		{
			ExecuteRenderQueue();
			BlockRenderThread();
			ExecuteRenderQueue();
			BlockRenderThread();
		};
		m_Pool.EraseThread(0);
		for (uint32_t i = 0; i < m_FramesInFlight; ++i)
			ExecuteResourceQueue(i);

		delete[] m_ResourceQueues;
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

	void RendererQueueData::ExecuteResourceQueue(uint32_t index)
	{
		std::scoped_lock lock(m_ResourceQueues[index].Mutex);
		m_ResourceQueues[index].Queue.Execute();
	}

	void RendererQueueData::BlockRenderThread()
	{
		#ifdef RENDER_THREAD_ENABLED
		XYZ_PROFILE_FUNC("RendererQueueData::BlockRenderThread");
		if (m_RenderThreadFinished.valid())
			m_RenderThreadFinished.wait();
		#endif
	}

	RenderCommandQueue& RendererQueueData::GetRenderCommandQueue()
	{
		return m_RenderCommandQueue[m_RenderWriteIndex];
	}

	ScopedLock<RenderCommandQueue> RendererQueueData::GetResourceQueue(uint32_t index)
	{
		return ScopedLock<RenderCommandQueue>(&m_ResourceQueues[index].Mutex, m_ResourceQueues[index].Queue);
	}

	bool RendererQueueData::RenderCommandQueuesEmpty() const
	{
		return m_RenderCommandQueue[0].GetCommandCount() == 0 && m_RenderCommandQueue[1].GetCommandCount() == 0;
	}
}