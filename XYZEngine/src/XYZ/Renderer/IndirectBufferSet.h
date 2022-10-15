#pragma once
#include "Buffer.h"
#include "Shader.h"

namespace XYZ {
	class IndirectBufferSet : public RefCount
	{
	public:
		virtual void CreateDescriptors(const Ref<Shader>& shader) = 0;
		virtual void Create(uint32_t size, uint32_t set, uint32_t binding) = 0;
		virtual void Set(Ref<IndirectBuffer> indirectBuffer, uint32_t set = 0, uint32_t frame = 0) = 0;
		
		virtual Ref<IndirectBuffer> Get(uint32_t binding, uint32_t set = 0, uint32_t frame = 0) = 0;
		
		static Ref<IndirectBufferSet> Create(uint32_t frames);
	};
}