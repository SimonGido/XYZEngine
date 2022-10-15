#pragma once

#include "VulkanIndirectBuffer.h"
#include "VulkanShader.h"

#include "XYZ/Renderer/IndirectBufferSet.h"


namespace XYZ {
	class VulkanIndirectBufferSet : public IndirectBufferSet
	{
	public:
		VulkanIndirectBufferSet(uint32_t frames);

		virtual void CreateDescriptors(const Ref<Shader>& shader) override;
		virtual void Create(uint32_t size, uint32_t set, uint32_t binding) override;
		virtual void Set(Ref<IndirectBuffer> indirectBuffer, uint32_t set = 0, uint32_t frame = 0) override;
		
		virtual Ref<IndirectBuffer> Get(uint32_t binding, uint32_t set = 0, uint32_t frame = 0) override;

		const std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, Ref<VulkanIndirectBuffer>>>>& GetIndirectBuffers() const { return m_IndirectBuffers; }

		const std::vector<std::vector<std::vector<VkWriteDescriptorSet>>>& GetDescriptors(size_t hash) const;
		const std::vector<std::vector<std::vector<VkWriteDescriptorSet>>>& GetDescriptors(const Ref<Shader>& shader);
	private:
		void RT_createDescriptors(const Ref<VulkanShader>& shader);

	private:
		uint32_t m_Frames;

		// shader hash -> per frame write data -> per set
		std::unordered_map<size_t, std::vector<std::vector<std::vector<VkWriteDescriptorSet>>>> m_WriteDescriptors;

		// frame->set->binding
		std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, Ref<VulkanIndirectBuffer>>>> m_IndirectBuffers;
	};
}