#pragma once
#include "Buffer.h"

#include <map>

namespace XYZ {
	class UniformBufferSet : public RefCount
	{
	public:
		UniformBufferSet(uint32_t frames);
		virtual ~UniformBufferSet() override;

		void Create(uint32_t size, uint32_t set, uint32_t binding);
		void Set(Ref<UniformBuffer> uniformBuffer, uint32_t set = 0, uint32_t frame = 0);
		
		Ref<UniformBuffer> Get(uint32_t binding, uint32_t set = 0, uint32_t frame = 0);
	
	private:
		uint32_t m_Frames;
		std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, Ref<UniformBuffer>>>> m_UniformBuffers; // frame->set->binding
	};
}