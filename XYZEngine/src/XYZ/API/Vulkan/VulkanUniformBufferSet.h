#pragma once
#include "XYZ/Renderer/UniformBufferSet.h"
#include "VulkanUniformBuffer.h"
#include "Vulkan.h"

namespace XYZ {
	class VulkanUniformBufferSet : public UniformBufferSet
	{
	public:
		VulkanUniformBufferSet(uint32_t frames);

		virtual void CreateDescriptors(const Ref<Shader>& shader) override;
		virtual void Create(uint32_t size, uint32_t set, uint32_t binding) override;
		virtual void Set(Ref<UniformBuffer> uniformBuffer, uint32_t set = 0, uint32_t frame = 0) override;

		virtual Ref<UniformBuffer> Get(uint32_t binding, uint32_t set = 0, uint32_t frame = 0) override;

		const std::vector<std::vector<std::vector<VkWriteDescriptorSet>>> & GetDescriptors(size_t hash) const;
	private:
		uint32_t m_Frames;

		// shader hash -> per frame write data -> per set
		std::unordered_map<size_t, std::vector<std::vector<std::vector<VkWriteDescriptorSet>>>> m_WriteDescriptors;
		
		// frame->set->binding
		std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, Ref<VulkanUniformBuffer>>>> m_UniformBuffers; 
	};
}