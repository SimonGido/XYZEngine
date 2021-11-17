#include "stdafx.h"
#include "UniformBufferSet.h"


namespace XYZ {
	UniformBufferSet::UniformBufferSet(uint32_t frames)
		:
		m_Frames(frames)
	{
	}
	UniformBufferSet::~UniformBufferSet()
	{
	}
	void UniformBufferSet::Create(uint32_t size, uint32_t set, uint32_t binding)
	{
		for (uint32_t frame = 0; frame < m_Frames; frame++)
		{
			Ref<UniformBuffer> uniformBuffer = UniformBuffer::Create(size, binding);
			Set(uniformBuffer, set, frame);
		}
	}
	void UniformBufferSet::Set(Ref<UniformBuffer> uniformBuffer, uint32_t set, uint32_t frame)
	{
		m_UniformBuffers[frame][set][uniformBuffer->GetBinding()] = uniformBuffer;
	}
	Ref<UniformBuffer> UniformBufferSet::Get(uint32_t binding, uint32_t set, uint32_t frame)
	{
		XYZ_ASSERT(m_UniformBuffers.find(frame) != m_UniformBuffers.end(), "");
		XYZ_ASSERT(m_UniformBuffers.at(frame).find(set) != m_UniformBuffers.at(frame).end(), "");
		XYZ_ASSERT(m_UniformBuffers.at(frame).at(set).find(binding) != m_UniformBuffers.at(frame).at(set).end(), "");

		return m_UniformBuffers.at(frame).at(set).at(binding);
	}
}