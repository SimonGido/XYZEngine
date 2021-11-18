#pragma once
#include "XYZ/Renderer/UniformBufferSet.h"
#include "VulkanUniformBuffer.h"
#include "Vulkan.h"

namespace XYZ {
	class VulkanUniformBufferSet : public UniformBufferSet
	{
	public:
		struct ShaderWriteDescriptorSet
		{
			std::vector<VkWriteDescriptorSet> WriteDescriptors;
			std::vector<VkDescriptorSet>	  DescriptorSets;
		};
	public:
		VulkanUniformBufferSet(uint32_t frames);

		virtual void CreateDescriptors(const Ref<Shader>& shader) override;
		virtual void Create(uint32_t size, uint32_t set, uint32_t binding) override;
		virtual void Set(Ref<UniformBuffer> uniformBuffer, uint32_t set = 0, uint32_t frame = 0) override;

		virtual Ref<UniformBuffer> Get(uint32_t binding, uint32_t set = 0, uint32_t frame = 0) override;

		const ShaderWriteDescriptorSet & GetDescriptors(size_t hash, uint32_t frame) const;
	private:
		uint32_t m_Frames;

		// shader hash -> per frame write data
		std::unordered_map<size_t, std::vector<ShaderWriteDescriptorSet>> m_WriteDescriptors;
		
		// frame->set->binding
		std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, Ref<VulkanUniformBuffer>>>> m_UniformBuffers; 
	};
}