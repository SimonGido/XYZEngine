#include "stdafx.h"
#include "VulkanFence.h"


namespace XYZ {
	VulkanFence::VulkanFence(uint64_t timeOut)
		:
		m_Timeout(timeOut)
	{
	}
}