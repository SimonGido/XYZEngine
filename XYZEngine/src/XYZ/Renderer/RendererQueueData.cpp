#include "stdafx.h"
#include "RendererQueueData.h"

#include "RendererAPI.h"
#include "Fence.h"

#include "XYZ/Core/Application.h"
#include "XYZ/Debug/Profiler.h"
#include "XYZ/Debug/Timer.h"


namespace XYZ {
	void RendererQueueData::Init(uint32_t framesInFlight)
	{
		m_FramesInFlight = framesInFlight;
		m_Pool.Start(1);
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
		m_Pool.Stop();
		for (uint32_t i = 0; i < m_FramesInFlight; ++i)
			ExecuteResourceQueue(i);

#ifdef XYZ_DEBUG
		XYZ_ASSERT(m_RenderCommandQueue[0].GetCommandCount() == 0, "");
		XYZ_ASSERT(m_RenderCommandQueue[1].GetCommandCount() == 0, "");

		for (uint32_t i = 0; i < m_FramesInFlight; ++i)
		{
			XYZ_ASSERT(m_ResourceQueues[i].Queue.GetCommandCount() == 0, "");
		}
#endif
		delete[] m_ResourceQueues;
	}

	void RendererQueueData::ExecuteRenderQueue()
	{
		RenderCommandQueue* queue = &m_RenderCommandQueue[m_RenderWriteIndex];
		#ifdef RENDER_THREAD_ENABLED

		m_RenderWriteIndex = m_RenderWriteIndex == 0 ? 1 : 0;
		m_RenderThreadFinished = m_Pool.SubmitJob([this, queue]() -> bool{
			XYZ_PROFILE_FUNC("RendererQueueData::ExecuteRenderQueue Job");
			XYZ_SCOPE_PERF("RendererQueueData::ExecuteRenderQueue");

			queue->Execute();
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
		XYZ_SCOPE_PERF("RendererQueueData::BlockRenderThread");

		if (m_RenderThreadFinished.valid())
			m_RenderThreadFinished.wait();
		#endif
	}

	ScopedLock<RenderCommandQueue> RendererQueueData::GetRenderCommandQueue()
	{
		return ScopedLock<RenderCommandQueue>(&m_RenderCommandQueueMutex, m_RenderCommandQueue[m_RenderWriteIndex]);
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