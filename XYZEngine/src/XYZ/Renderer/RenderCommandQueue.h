#pragma once
#include <tuple>
#include <mutex>

namespace XYZ {

	class RenderCommandQueue
	{
	public:
		typedef void(*RenderCommandFn)(void*);

		RenderCommandQueue(uint32_t size = 10 * 1024 * 1024);
		~RenderCommandQueue();

		void* Allocate(RenderCommandFn func, uint32_t size);

		void Execute();

		uint32_t GetCommandCount() const { return m_CommandCount; }
	private:
		uint8_t* m_CommandBuffer;
		uint8_t* m_CommandBufferPtr;
		uint32_t m_CommandCount = 0;
		uint32_t m_Size;
	};
}