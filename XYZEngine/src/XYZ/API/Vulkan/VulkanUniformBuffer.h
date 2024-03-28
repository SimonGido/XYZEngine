#pragma once
#include "XYZ/Utils/DataStructures/ByteBuffer.h"
#include "XYZ/Renderer/Buffer.h"
#include "VulkanAllocator.h"
#include "VulkanShader.h"

namespace XYZ {

	class VulkanUniformBuffer : public UniformBuffer
	{
	public:
		VulkanUniformBuffer(uint32_t size, uint32_t binding);
		virtual ~VulkanUniformBuffer() override;

		virtual void Update(const void* data, uint32_t size, uint32_t offset = 0) override;
		virtual void RT_Update(const void* data, uint32_t size, uint32_t offset = 0) override;
		virtual uint32_t GetBinding() const override { return m_Binding; }

		void RT_SetBufferInfo(uint32_t size, uint32_t offset);
		const VkDescriptorBufferInfo& GetDescriptorBufferInfo() const { return m_DescriptorInfo; }
	private:
		void RT_invalidate();
		void release();

	private:
		VmaAllocation		   m_MemoryAlloc;
		VkBuffer			   m_Buffer;
		VkDescriptorBufferInfo m_DescriptorInfo{};
		std::string			   m_Name;
		VkShaderStageFlagBits  m_ShaderStage;
		ByteBuffer			   m_LocalStorage;
		uint32_t			   m_Binding;

	};
}