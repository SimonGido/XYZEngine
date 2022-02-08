#pragma once
#include "XYZ/Renderer/Shader.h"
#include "Buffer.h"

#include <map>

namespace XYZ {
	class UniformBufferSet : public RefCount
	{
	public:
		virtual void Create(uint32_t size, uint32_t set, uint32_t binding) = 0;
		virtual void Set(Ref<UniformBuffer> uniformBuffer, uint32_t set = 0, uint32_t frame = 0) = 0;
		
		virtual void CreateDescriptors(const Ref<Shader>& shader) = 0;

		virtual Ref<UniformBuffer> Get(uint32_t binding, uint32_t set = 0, uint32_t frame = 0) = 0;
		virtual bool HasDescriptors(size_t hash) const = 0;

		static Ref<UniformBufferSet> Create(uint32_t frames);
	};
}