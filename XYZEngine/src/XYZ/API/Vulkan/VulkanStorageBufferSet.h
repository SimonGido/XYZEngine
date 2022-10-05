#pragma once
#include "XYZ/Renderer/StorageBufferSet.h"
#include "VulkanStorageBuffer.h"
#include "VulkanShader.h"
#include "Vulkan.h"

namespace XYZ {

	class VulkanStorageBufferSet : public StorageBufferSet
	{
	public:
		VulkanStorageBufferSet(uint32_t frames);
		virtual void Update(const void* data, uint32_t size, uint32_t offset, uint32_t binding, uint32_t set = 0) override;
		virtual void CreateDescriptors(const Ref<Shader>& shader) override;
		virtual void Create(uint32_t size, uint32_t set, uint32_t binding) override;
		virtual void Set(Ref<StorageBuffer> storageBuffer, uint32_t set = 0, uint32_t frame = 0) override;
		virtual void Resize(uint32_t size, uint32_t set, uint32_t binding) override;

		virtual Ref<StorageBuffer> Get(uint32_t binding, uint32_t set = 0, uint32_t frame = 0) override;

		const std::vector<std::vector<std::vector<VkWriteDescriptorSet>>>& GetDescriptors(size_t hash) const;
		const std::vector<std::vector<std::vector<VkWriteDescriptorSet>>>& GetDescriptors(const Ref<Shader>& shader);
	private:
		void RT_createDescriptors(const Ref<VulkanShader>& shader);

	private:
		uint32_t m_Frames;

		// shader hash -> per frame write data -> per set
		std::unordered_map<size_t, std::vector<std::vector<std::vector<VkWriteDescriptorSet>>>> m_WriteDescriptors;

		// frame->set->binding
		std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, Ref<VulkanStorageBuffer>>>> m_StorageBuffers;
	};
}