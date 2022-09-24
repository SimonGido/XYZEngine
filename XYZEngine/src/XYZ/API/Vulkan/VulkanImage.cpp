#include "stdafx.h"
#include "VulkanImage.h"

#include "VulkanContext.h"
#include "VulkanAllocator.h"
#include "VulkanRendererAPI.h"

#include "XYZ/Renderer/Renderer.h"

namespace XYZ {
	VulkanImage2D::VulkanImage2D(const ImageSpecification& specification)
		: m_Specification(specification)
	{
	}
	VulkanImage2D::~VulkanImage2D()
	{
		m_ImageData.Destroy();
		auto info = m_Info;
		auto imageViews = m_PerLayerImageViews;
		for (auto& mipView : m_MipImageViews)
			imageViews.push_back(mipView.View);

		Renderer::SubmitResource([info, imageViews]() mutable {
			RT_release(info, imageViews);
		});
	}
	void VulkanImage2D::Invalidate()
	{
		Ref<VulkanImage2D> instance = this;
		Renderer::Submit([instance]() mutable
		{
			instance->RT_Invalidate();
		});
	}
	void VulkanImage2D::Release()
	{
		Ref<VulkanImage2D> instance = this;
		Renderer::SubmitResource([instance]() mutable
		{
			RT_release(instance->m_Info, instance->m_PerLayerImageViews);
			instance->m_Info.Image = VK_NULL_HANDLE;
			instance->m_Info.ImageView = VK_NULL_HANDLE;
			instance->m_Info.Sampler = VK_NULL_HANDLE;
			instance->m_PerLayerImageViews.clear();
			instance->m_PerLayerImageViews.clear();
			instance->m_MipImageViews.clear();
		});
		
	}
	void VulkanImage2D::CreatePerLayerImageViews()
	{
		Ref<VulkanImage2D> instance = this;
		Renderer::Submit([instance]() mutable
		{
			instance->RT_CreatePerLayerImageViews();
		});
	}
	VkImageView VulkanImage2D::GetLayerImageView(uint32_t layer)
	{
		XYZ_ASSERT(layer < m_PerLayerImageViews.size(), "");
		return m_PerLayerImageViews[layer];
	}
	void VulkanImage2D::RT_Invalidate()
	{
		UpdateDescriptor();
		VulkanImageInfo oldInfo = m_Info;
		auto perLayerImageViews = m_PerLayerImageViews;
		Renderer::SubmitResource([oldInfo, perLayerImageViews]() {
			RT_release(oldInfo, perLayerImageViews);
		});

		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
		VulkanAllocator allocator("Image2D");
	
		m_Info.MemoryAlloc = allocator.AllocateImage(createImageCreateInfo(), VMA_MEMORY_USAGE_GPU_ONLY, m_Info.Image);
		if (m_ImageData)
			copyImageData();

		
		if (m_Specification.Usage == ImageUsage::Storage)
		{
			// Transition image to GENERAL layout
			VkCommandBuffer commandBuffer = VulkanContext::GetCurrentDevice()->GetCommandBuffer(true);

			VkImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = m_Specification.Mips;
			subresourceRange.layerCount = m_Specification.Layers;
			VulkanRendererAPI::SetImageLayout(commandBuffer, m_Info.Image, 
				VK_IMAGE_LAYOUT_UNDEFINED, m_DescriptorImageInfo.imageLayout, 
				subresourceRange
			);
			VulkanContext::GetCurrentDevice()->FlushCommandBuffer(commandBuffer);
		}
		
		VkImageViewCreateInfo imageViewCreateInfo = createImageViewCreateInfo(
			m_Specification.Layers, 0, m_Specification.Mips, 0
		);
		imageViewCreateInfo.viewType = m_Specification.Layers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
		VK_CHECK_RESULT(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_Info.ImageView));
		
		VkSamplerCreateInfo samplerCreateInfo = createSamplerCreateInfo();
		VK_CHECK_RESULT(vkCreateSampler(device, &samplerCreateInfo, nullptr, &m_Info.Sampler));

		UpdateDescriptor();
	}
	void VulkanImage2D::RT_CreatePerLayerImageViews()
	{
		XYZ_ASSERT(m_Specification.Layers > 1, "");
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

		m_PerLayerImageViews.resize(m_Specification.Layers);
		for (uint32_t layer = 0; layer < m_Specification.Layers; layer++)
		{
			VkImageViewCreateInfo imageViewCreateInfo = createImageViewCreateInfo(1, layer, m_Specification.Mips, 0);
			VK_CHECK_RESULT(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_PerLayerImageViews[layer]));
		}
	}
	void VulkanImage2D::RT_CreatePerSpecificLayerImageViews(const std::vector<uint32_t>& layerIndices)
	{
		XYZ_ASSERT(m_Specification.Layers > 1, "");

		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
		const VkFormat vulkanFormat = Utils::VulkanImageFormat(m_Specification.Format);

		if (m_PerLayerImageViews.empty())
			m_PerLayerImageViews.resize(m_Specification.Layers);

		for (uint32_t layer : layerIndices)
		{
			VkImageViewCreateInfo imageViewCreateInfo = createImageViewCreateInfo(1, layer, m_Specification.Mips, 0);
			VK_CHECK_RESULT(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_PerLayerImageViews[layer]));
		}
	}

	VkImageView VulkanImage2D::GetMipImageView(uint32_t mip)
	{
		if (mip >= m_MipImageViews.size() || !m_MipImageViews[mip].Created)
		{
			Ref<VulkanImage2D> instance = this;
			Renderer::Submit([instance, mip]() mutable
			{
				instance->RT_GetMipImageView(mip);
			});
		}
		return m_MipImageViews[mip].View;
	}
	VkImageView VulkanImage2D::RT_GetMipImageView(uint32_t mip)
	{
		if (mip >= m_MipImageViews.size() || !m_MipImageViews[mip].Created)
		{
			if (mip >= m_MipImageViews.size())
				m_MipImageViews.resize(static_cast<size_t>(mip) + 1);

			VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
			VkImageViewCreateInfo imageViewCreateInfo = createImageViewCreateInfo(1, 0, 1, mip);

			auto& mipView = m_MipImageViews[mip];
			VK_CHECK_RESULT(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &mipView.View));
			mipView.Created = true;
			mipView.Descriptor = m_DescriptorImageInfo;
			mipView.Descriptor.imageView = mipView.View;
		}
		return m_MipImageViews[mip].View;
	}
	const VkDescriptorImageInfo& VulkanImage2D::RT_GetMipImageDescriptor(uint32_t mip)
	{
		RT_GetMipImageView(mip);
		return m_MipImageViews[mip].Descriptor;
	}
	
	void VulkanImage2D::RT_release(const VulkanImageInfo& info, const std::vector<VkImageView>& views)
	{
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
		if (info.Image == nullptr)
			return;
		
		vkDestroyImageView(device, info.ImageView, nullptr);
		vkDestroySampler(device, info.Sampler, nullptr);

		for (auto& view : views)
		{
			if (view)
				vkDestroyImageView(device, view, nullptr);
		}
		VulkanAllocator allocator("VulkanImage2D");
		allocator.DestroyImage(info.Image, info.MemoryAlloc);
	}
	void VulkanImage2D::copyImageData()
	{
		// Create staging buffer
		VulkanAllocator allocator("Image2D");
		VkBuffer stagingBuffer;
		VmaAllocation stagingBufferAllocation = allocateStagingBuffer(allocator, stagingBuffer);

		// Copy data to staging buffer
		uint8_t* destData = allocator.MapMemory<uint8_t>(stagingBufferAllocation);
		XYZ_ASSERT(m_ImageData.Data, "");
		memcpy(destData, m_ImageData.Data, m_ImageData.Size);
		allocator.UnmapMemory(stagingBufferAllocation);

		auto device = VulkanContext::GetCurrentDevice();
		VkCommandBuffer copyCmd = device->GetCommandBuffer(true);

		VulkanRendererAPI::SetImageLayout(
			copyCmd, m_Info.Image, VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_HOST_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT
		);

		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = m_Specification.Width;
		bufferCopyRegion.imageExtent.height = m_Specification.Height;
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = 0;

		vkCmdCopyBufferToImage(
			copyCmd,
			stagingBuffer,
			m_Info.Image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&bufferCopyRegion
		);

		VkImageSubresourceRange subresourceRange = {};
		// Image only contains color data
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		// Start at first mip level
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;

		if (m_Specification.Mips > 1)
		{
			VulkanRendererAPI::InsertImageMemoryBarrier(copyCmd, m_Info.Image,
				VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				subresourceRange
			);
		}
		else
		{
			VulkanRendererAPI::InsertImageMemoryBarrier(copyCmd, m_Info.Image,
				VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_DescriptorImageInfo.imageLayout,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				subresourceRange
			);
		}
		device->FlushCommandBuffer(copyCmd);
		allocator.DestroyBuffer(stagingBuffer, stagingBufferAllocation);
	}
	VmaAllocation VulkanImage2D::allocateStagingBuffer(VulkanAllocator& allocator, VkBuffer& stagingBuffer) const
	{
		VkDeviceSize size = m_ImageData.Size;

		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		return allocator.AllocateBuffer(bufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);
	}
	VkImageUsageFlags VulkanImage2D::findUsage() const
	{
		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT; // TODO: this (probably) shouldn't be implied
		if (m_Specification.Usage == ImageUsage::Attachment)
		{
			if (Utils::IsDepthFormat(m_Specification.Format))
				usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			else
				usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
		else if (m_Specification.Usage == ImageUsage::Texture)
		{
			usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
		else if (m_Specification.Usage == ImageUsage::Storage)
		{
			usage |= VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
		return usage;
	}
	VkImageCreateInfo VulkanImage2D::createImageCreateInfo() const
	{
		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = Utils::VulkanImageFormat(m_Specification.Format);
		imageCreateInfo.extent.width = m_Specification.Width;
		imageCreateInfo.extent.height = m_Specification.Height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = m_Specification.Mips;
		imageCreateInfo.arrayLayers = m_Specification.Layers;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = findUsage();
		return imageCreateInfo;
	}
	VkImageViewCreateInfo VulkanImage2D::createImageViewCreateInfo(uint32_t layerCount, uint32_t baseLayer, uint32_t mipCount, uint32_t baseMip) const
	{
		VkImageAspectFlags aspectMask = Utils::IsDepthFormat(m_Specification.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (m_Specification.Format == ImageFormat::DEPTH24STENCIL8)
			aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = Utils::VulkanImageFormat(m_Specification.Format);
		imageViewCreateInfo.flags = 0;
		imageViewCreateInfo.subresourceRange = {};
		imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
		imageViewCreateInfo.subresourceRange.baseMipLevel = baseMip;
		imageViewCreateInfo.subresourceRange.levelCount = mipCount;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = baseLayer;
		imageViewCreateInfo.subresourceRange.layerCount = layerCount;
		imageViewCreateInfo.image = m_Info.Image;

		return imageViewCreateInfo;
	}
	VkSamplerCreateInfo VulkanImage2D::createSamplerCreateInfo() const
	{
		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
		samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 100.0f;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		return samplerCreateInfo;
	}
	void VulkanImage2D::UpdateDescriptor()
	{	
		if (Utils::IsDepthFormat(m_Specification.Format) && m_Specification.Usage != ImageUsage::Storage)
			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		else if (m_Specification.Usage == ImageUsage::Storage)
			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		else
			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		m_DescriptorImageInfo.imageView = m_Info.ImageView;
		m_DescriptorImageInfo.sampler = m_Info.Sampler;
	}
}