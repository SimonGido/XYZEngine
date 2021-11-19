#pragma once
#include "XYZ/Renderer/Fence.h"


namespace XYZ {
	class VulkanFence : public Fence
	{
	public:
		VulkanFence(uint64_t timeOut);


	private:
		uint64_t m_Timeout;
	};
}