#pragma once
#include "XYZ/Renderer/Shader.h"
#include "Buffer.h"

#include <map>

namespace XYZ {
	class StorageBufferSet : public RefCount
	{
	public:
		virtual void Update(const void* data, uint32_t size, uint32_t offset, uint32_t binding, uint32_t set = 0) = 0;
		virtual void Create(uint32_t size, uint32_t set, uint32_t binding, bool indirect = false) = 0;
	
		virtual void Set(Ref<StorageBuffer> storageBuffer, uint32_t set = 0, uint32_t frame = 0) = 0;
		virtual void Resize(uint32_t size, uint32_t set, uint32_t binding) = 0;

		virtual void CreateDescriptors(const Ref<Shader>& shader) = 0;

		virtual Ref<StorageBuffer> Get(uint32_t binding, uint32_t set = 0, uint32_t frame = 0) = 0;
	
		static Ref<StorageBufferSet> Create(uint32_t frames);
	};
}