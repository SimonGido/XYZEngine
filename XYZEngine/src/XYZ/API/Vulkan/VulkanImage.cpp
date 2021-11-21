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
		Release();
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
		if (m_Info.Image == nullptr)
			return;

		Renderer::SubmitResource([info = m_Info, layerViews = m_PerLayerImageViews]() mutable
		{
			const auto vulkanDevice = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
			vkDestroyImageView(vulkanDevice, info.ImageView, nullptr);
			vkDestroySampler(vulkanDevice, info.Sampler, nullptr);

			for (auto& view : layerViews)
			{
				if (view)
					vkDestroyImageView(vulkanDevice, view, nullptr);
			}
			VulkanAllocator allocator("VulkanImage2D");
			allocator.DestroyImage(info.Image, info.MemoryAlloc);
		});
		m_Info.Image	 = VK_NULL_HANDLE;
		m_Info.ImageView = VK_NULL_HANDLE;
		m_Info.Sampler   = VK_NULL_HANDLE;
		m_PerLayerImageViews.clear();
		m_PerLayerImageViews.clear();
		m_MipImageViews.clear();
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
	void VulkanImage2D::RT_Invalidate(bool createDefault)
	{
		Release();
		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
		VulkanAllocator allocator("Image2D");
	
		m_Info.MemoryAlloc = allocator.AllocateImage(createImageCreateInfo(), VMA_MEMORY_USAGE_GPU_ONLY, m_Info.Image);
		
		if (createDefault)
		{
			VkImageViewCreateInfo imageViewCreateInfo = createImageViewCreateInfo(
				m_Specification.Layers, 0, m_Specification.Mips, 0
			);
			imageViewCreateInfo.viewType = m_Specification.Layers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
			VK_CHECK_RESULT(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_Info.ImageView));

			VkSamplerCreateInfo samplerCreateInfo = createSamplerCreateInfo();
			VK_CHECK_RESULT(vkCreateSampler(device, &samplerCreateInfo, nullptr, &m_Info.Sampler));

			if (m_Specification.Usage == ImageUsage::Storage)
			{
				// Transition image to GENERAL layout
				VkCommandBuffer commandBuffer = VulkanContext::GetCurrentDevice()->GetCommandBuffer(true);

				VkImageSubresourceRange subresourceRange = {};
				subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				subresourceRange.baseMipLevel = 0;
				subresourceRange.levelCount = m_Specification.Mips;
				subresourceRange.layerCount = m_Specification.Layers;

				VulkanRendererAPI::InsertImageMemoryBarrier(commandBuffer, m_Info.Image,
					0, 0,
					VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					subresourceRange);

				VulkanContext::GetCurrentDevice()->FlushCommandBuffer(commandBuffer);
			}
			UpdateDescriptor();
		}
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

		VkImageAspectFlags aspectMask = Utils::IsDepthFormat(m_Specification.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (m_Specification.Format == ImageFormat::DEPTH24STENCIL8)
			aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

		const VkFormat vulkanFormat = Utils::VulkanImageFormat(m_Specification.Format);

		//HZ_CORE_ASSERT(m_PerLayerImageViews.size() == m_Specification.Layers);
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
				m_MipImageViews.resize(mip + 1);

			VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
			VkImageViewCreateInfo imageViewCreateInfo = createImageViewCreateInfo(1, 0, 1, mip);

			VK_CHECK_RESULT(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_MipImageViews[mip].View));
			m_MipImageViews[mip].Created = true;
		}
		return m_MipImageViews[mip].View;
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
		if (m_Specification.Format == ImageFormat::DEPTH24STENCIL8 || m_Specification.Format == ImageFormat::DEPTH32F)
			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		else if (m_Specification.Usage == ImageUsage::Storage)
			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		else
			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		if (m_Specification.Usage == ImageUsage::Storage)
			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		m_DescriptorImageInfo.imageView = m_Info.ImageView;
		m_DescriptorImageInfo.sampler = m_Info.Sampler;
	}
}