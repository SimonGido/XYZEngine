#pragma once
#include "XYZ/Renderer/Shader.h"
#include "Buffer.h"

#include <map>

namespace XYZ {
	struct UniformBufferInfo
	{
		uint32_t Offset;
		uint32_t Size;
	};

	class XYZ_API UniformBufferSet : public RefCount
	{
	public:
		virtual ~UniformBufferSet() = default;

		virtual void Create(uint32_t size, uint32_t set, uint32_t binding) = 0;
		virtual void Set(Ref<UniformBuffer> uniformBuffer, uint32_t set = 0, uint32_t frame = 0) = 0;
		virtual void UpdateEachFrame(const void* data, uint32_t size, uint32_t offset, uint32_t binding, uint32_t set = 0) = 0;
		virtual void CreateDescriptors(const Ref<Shader>& shader) = 0;

		virtual Ref<UniformBuffer> Get(uint32_t binding, uint32_t set = 0, uint32_t frame = 0) = 0;
		virtual bool HasDescriptors(size_t hash) const = 0;

		virtual void SetBufferInfo(uint32_t size, uint32_t offset, uint32_t binding, uint32_t set = 0) = 0;

		static Ref<UniformBufferSet> Create(uint32_t frames);
	};
}