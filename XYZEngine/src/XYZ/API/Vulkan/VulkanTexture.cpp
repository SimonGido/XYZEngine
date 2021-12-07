#include "stdafx.h"
#include "VulkanTexture.h"
#include "VulkanContext.h"
#include "VulkanRendererAPI.h"

#include <stb_image.h>

namespace XYZ {
	namespace Utils {

		static VkSamplerAddressMode VulkanSamplerWrap(TextureWrap wrap)
		{
			switch (wrap)
			{
			case TextureWrap::Clamp:   return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case TextureWrap::Repeat:  return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			}
			XYZ_ASSERT(false, "Unknown wrap mode");
			return (VkSamplerAddressMode)0;
		}

		static VkFilter VulkanSamplerFilter(TextureFilter filter)
		{
			switch (filter)
			{
			case TextureFilter::Linear:   return VK_FILTER_LINEAR;
			case TextureFilter::Nearest:  return VK_FILTER_NEAREST;
			}
			XYZ_ASSERT(false, "Unknown filter");
			return (VkFilter)0;
		}

		static size_t GetMemorySize(ImageFormat format, uint32_t width, uint32_t height)
		{
			switch (format)
			{
			case ImageFormat::RGBA:    return width * height * 4;
			case ImageFormat::RGBA32F: return width * height * 4 * sizeof(float);
			}
			XYZ_ASSERT(false, "");
			return 0;
		}

	}
	VulkanTexture2D::VulkanTexture2D(const std::string& path, const TextureProperties& properties)
		: m_Path(path), m_Properties(properties)
	{
		ImageSpecification imageSpec;
		imageSpec.Format = m_Format;
		imageSpec.Width = m_Width;
		imageSpec.Height = m_Height;
		imageSpec.Mips = GetMipLevelCount();
		imageSpec.DebugName = properties.DebugName;
		m_Image = Image2D::Create(imageSpec);
		ByteBuffer& imageBuffer = m_Image->GetBuffer();
		loadImage(path, imageBuffer);
		XYZ_ASSERT(imageBuffer, "");
		XYZ_ASSERT(m_Format != ImageFormat::None, "");

		Invalidate();
		
	}
	VulkanTexture2D::VulkanTexture2D(ImageFormat format, uint32_t width, uint32_t height, const void* data, const TextureProperties& properties)
		: m_Properties(properties), m_Format(format)
	{
		m_Width = width;
		m_Height = height;

		uint32_t size = Utils::GetMemorySize(format, width, height);
	
		ImageSpecification imageSpec;
		imageSpec.Format = m_Format;
		imageSpec.Width = m_Width;
		imageSpec.Height = m_Height;
		imageSpec.Mips = GetMipLevelCount();
		imageSpec.DebugName = properties.DebugName;
		if (properties.Storage)
			imageSpec.Usage = ImageUsage::Storage;
		m_Image = Image2D::Create(imageSpec);
		
		if (data)
		{
			ByteBuffer& imageBuffer = m_Image->GetBuffer();
			imageBuffer = ByteBuffer::Copy(data, size);
		}
		Invalidate();
	}

	VulkanTexture2D::~VulkanTexture2D()
	{
	}

	void VulkanTexture2D::Resize(uint32_t width, uint32_t height)
	{
		m_Width = width;
		m_Height = height;
		Invalidate();
	}
	std::pair<uint32_t, uint32_t> VulkanTexture2D::GetMipSize(uint32_t mip) const
	{
		return std::pair<uint32_t, uint32_t>();
	}
	void VulkanTexture2D::Invalidate()
	{
		Ref<VulkanTexture2D> instance = this;
		Renderer::Submit([instance]() mutable
		{
			instance->RT_Invalidate();
		});
	}
	void VulkanTexture2D::RT_Invalidate()
	{
		auto device = VulkanContext::GetCurrentDevice();
		auto vulkanDevice = device->GetVulkanDevice();

		const uint32_t mipCount = m_Properties.GenerateMips ? GetMipLevelCount() : 1;
		ImageSpecification& imageSpec = m_Image->GetSpecification();
		imageSpec.Format = m_Format;
		imageSpec.Width  = m_Width;
		imageSpec.Height = m_Height;
		imageSpec.Mips   = mipCount;
		if (m_Properties.Storage)
			imageSpec.Usage = ImageUsage::Storage;
		
		Ref<VulkanImage2D> vulkanImage = m_Image.As<VulkanImage2D>();
		vulkanImage->RT_Invalidate();
		

		if (m_Image->GetBuffer() && m_Properties.GenerateMips && mipCount > 1)
			GenerateMips();
	}
	void VulkanTexture2D::Lock()
	{
	}
	void VulkanTexture2D::Unlock()
	{
	}
	void VulkanTexture2D::GenerateMips()
	{
		auto device = VulkanContext::GetCurrentDevice();
		auto vulkanDevice = device->GetVulkanDevice();

		Ref<VulkanImage2D> image = m_Image.As<VulkanImage2D>();
		const auto& info = image->GetImageInfo();

		const VkCommandBuffer blitCmd = VulkanContext::GetCurrentDevice()->GetCommandBuffer(true);

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = info.Image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		const auto mipLevels = GetMipLevelCount();
		for (uint32_t i = 1; i < mipLevels; i++)
		{
			VkImageBlit imageBlit{};

			// Source
			imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlit.srcSubresource.layerCount = 1;
			imageBlit.srcSubresource.mipLevel = i - 1;
			imageBlit.srcOffsets[1].x = int32_t(m_Width >> (i - 1));
			imageBlit.srcOffsets[1].y = int32_t(m_Height >> (i - 1));
			imageBlit.srcOffsets[1].z = 1;

			// Destination
			imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlit.dstSubresource.layerCount = 1;
			imageBlit.dstSubresource.mipLevel = i;
			imageBlit.dstOffsets[1].x = int32_t(m_Width >> i);
			imageBlit.dstOffsets[1].y = int32_t(m_Height >> i);
			imageBlit.dstOffsets[1].z = 1;

			VkImageSubresourceRange mipSubRange = {};
			mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			mipSubRange.baseMipLevel = i;
			mipSubRange.levelCount = 1;
			mipSubRange.layerCount = 1;

			// Prepare current mip level as image blit destination
			VulkanRendererAPI::InsertImageMemoryBarrier(blitCmd, info.Image,
				0, VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				mipSubRange
			);

			// Blit from previous level
			vkCmdBlitImage(
				blitCmd,
				info.Image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				info.Image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&imageBlit,
				VK_FILTER_LINEAR
			);

			// Prepare current mip level as image blit source for next level
			VulkanRendererAPI::InsertImageMemoryBarrier(blitCmd, info.Image,
				VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				mipSubRange
			);
		}

		// After the loop, all mip layers are in TRANSFER_SRC layout, so transition all to SHADER_READ
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.layerCount = 1;
		subresourceRange.levelCount = mipLevels;

		VulkanRendererAPI::InsertImageMemoryBarrier(blitCmd, info.Image,
			VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			subresourceRange);

		VulkanContext::GetCurrentDevice()->FlushCommandBuffer(blitCmd);
	}
	uint32_t VulkanTexture2D::GetMipLevelCount() const
	{
		return Utils::CalculateMipCount(m_Width, m_Height);
	}
	ByteBuffer VulkanTexture2D::GetWriteableBuffer()
	{
		return m_Image->GetBuffer();
	}
	
	void VulkanTexture2D::loadImage(const std::string& path, ByteBuffer& imageData)
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		if (stbi_is_hdr(path.c_str()))
		{
			imageData.Data = (uint8_t*)stbi_loadf(path.c_str(), &width, &height, &channels, 4);
			imageData.Size = width * height * 4 * sizeof(float);
			m_Format = ImageFormat::RGBA32F;
		}
		else
		{		
			imageData.Data = stbi_load(path.c_str(), &width, &height, &channels, 4);
			imageData.Size = width * height * 4;
			m_Format = ImageFormat::RGBA;
		}

		XYZ_ASSERT(imageData.Data, "Failed to load image!");
		if (!imageData.Data)
			return;

		m_Width = width;
		m_Height = height;
	}

	//VkSamplerCreateInfo VulkanTexture2D::createSamplerCreateInfo(uint32_t mipCount) const
	//{
	//	VkSamplerCreateInfo sampler{};
	//	sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	//	sampler.maxAnisotropy = 1.0f;
	//	sampler.magFilter = Utils::VulkanSamplerFilter(m_Properties.SamplerFilter);
	//	sampler.minFilter = Utils::VulkanSamplerFilter(m_Properties.SamplerFilter);
	//	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	//	sampler.addressModeU = Utils::VulkanSamplerWrap(m_Properties.SamplerWrap);
	//	sampler.addressModeV = Utils::VulkanSamplerWrap(m_Properties.SamplerWrap);
	//	sampler.addressModeW = Utils::VulkanSamplerWrap(m_Properties.SamplerWrap);
	//	sampler.mipLodBias = 0.0f;
	//	sampler.compareOp = VK_COMPARE_OP_NEVER;
	//	sampler.minLod = 0.0f;
	//	sampler.maxLod = (float)mipCount;
	//	sampler.maxAnisotropy = 1.0;
	//	sampler.anisotropyEnable = VK_FALSE;
	//	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	//	return sampler;
	//}
	//
	//VkImageViewCreateInfo VulkanTexture2D::createImageViewCreateInfo(uint32_t layerCount, uint32_t baseLayer, uint32_t mipCount, uint32_t baseMip) const
	//{
	//	VkImageViewCreateInfo view{};
	//	view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	//	view.viewType = VK_IMAGE_VIEW_TYPE_2D;
	//	view.format = Utils::VulkanImageFormat(m_Format);
	//	view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	//	// The subresource range describes the set of mip levels (and array layers) that can be accessed through this image view
	//	// It's possible to create multiple image views for a single image referring to different (and/or overlapping) ranges of the image
	//	view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//	view.subresourceRange.baseMipLevel = baseMip;
	//	view.subresourceRange.baseArrayLayer = baseLayer;
	//	view.subresourceRange.layerCount = layerCount;
	//	view.subresourceRange.levelCount = mipCount;
	//	return view;
	//}	
}