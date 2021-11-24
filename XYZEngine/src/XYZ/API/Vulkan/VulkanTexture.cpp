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
		bool loaded = loadImage(path);
		XYZ_ASSERT(loaded, "");
		if (loaded)
		{
			ImageSpecification imageSpec;
			imageSpec.Format = m_Format;
			imageSpec.Width = m_Width;
			imageSpec.Height = m_Height;
			imageSpec.Mips = GetMipLevelCount();
			imageSpec.DebugName = properties.DebugName;
			m_Image = Image2D::Create(imageSpec);

			XYZ_ASSERT(m_Format != ImageFormat::None, "");

			Invalidate();
		}
	}
	VulkanTexture2D::VulkanTexture2D(ImageFormat format, uint32_t width, uint32_t height, const void* data, const TextureProperties& properties)
		: m_Properties(properties), m_Format(format)
	{
		m_Width = width;
		m_Height = height;

		uint32_t size = Utils::GetMemorySize(format, width, height);

		if (data)
		{
			m_ImageData = ByteBuffer::Copy(data, size);
			memcpy(m_ImageData.m_Data, data, m_ImageData.m_Size);
		}

		ImageSpecification imageSpec;
		imageSpec.Format = m_Format;
		imageSpec.Width = m_Width;
		imageSpec.Height = m_Height;
		imageSpec.Mips = GetMipLevelCount();
		imageSpec.DebugName = properties.DebugName;
		if (properties.Storage)
			imageSpec.Usage = ImageUsage::Storage;
		m_Image = Image2D::Create(imageSpec);

		Invalidate();
	}
	VulkanTexture2D::~VulkanTexture2D()
	{
		m_ImageData.Destroy();
	}
	void VulkanTexture2D::SetData(const void* data, uint32_t size, uint32_t offset)
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

		m_Image->Release();
		const uint32_t mipCount = m_Properties.GenerateMips ? GetMipLevelCount() : 1;
		ImageSpecification& imageSpec = m_Image->GetSpecification();
		imageSpec.Format = m_Format;
		imageSpec.Width  = m_Width;
		imageSpec.Height = m_Height;
		imageSpec.Mips   = mipCount;
		
		Ref<VulkanImage2D> vulkanImage = m_Image.As<VulkanImage2D>();
		vulkanImage->RT_Invalidate(false);
		auto& info = vulkanImage->GetImageInfo();

		if (m_ImageData)
		{
			VkDeviceSize size = m_ImageData.m_Size;

			VkMemoryAllocateInfo memAllocInfo{};
			memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

			VulkanAllocator allocator("Texture2D");
			VkBuffer stagingBuffer;
			VmaAllocation stagingBufferAllocation = allocateStagingBuffer(allocator, stagingBuffer);
			
			// Copy data to staging buffer
			uint8_t* destData = allocator.MapMemory<uint8_t>(stagingBufferAllocation);
			XYZ_ASSERT(m_ImageData.m_Data, "");
			memcpy(destData, m_ImageData.m_Data, size);
			allocator.UnmapMemory(stagingBufferAllocation);

			VkCommandBuffer copyCmd = device->GetCommandBuffer(true);
			transitionToSrcLayout(copyCmd);
			copyBufferToImage(copyCmd, stagingBuffer);
			transitionToDstLayout(copyCmd, mipCount);
			device->FlushCommandBuffer(copyCmd);
			allocator.DestroyBuffer(stagingBuffer, stagingBufferAllocation);
		}
		else
		{
			VkCommandBuffer transitionCommandBuffer = device->GetCommandBuffer(true);
			VkImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.layerCount = 1;
			subresourceRange.levelCount = GetMipLevelCount();
			VulkanRendererAPI::SetImageLayout(transitionCommandBuffer, info.Image, VK_IMAGE_LAYOUT_UNDEFINED, vulkanImage->GetDescriptor().imageLayout, subresourceRange);
			device->FlushCommandBuffer(transitionCommandBuffer);
		}

		VkSamplerCreateInfo samplerCreateInfo = createSamplerCreateInfo(mipCount);
		VK_CHECK_RESULT(vkCreateSampler(vulkanDevice, &samplerCreateInfo, nullptr, &info.Sampler));
		if (!m_Properties.Storage)
		{
			VkImageViewCreateInfo view = createImageViewCreateInfo(1, 0, mipCount, 0);
			view.image = info.Image;
			VK_CHECK_RESULT(vkCreateImageView(vulkanDevice, &view, nullptr, &info.ImageView));
			vulkanImage->UpdateDescriptor();
		}
		if (m_ImageData && m_Properties.GenerateMips && mipCount > 1)
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
		return m_ImageData;
	}
	void VulkanTexture2D::transitionToSrcLayout(VkCommandBuffer copyCmd) const
	{
		Ref<VulkanImage2D> vulkanImage = m_Image.As<VulkanImage2D>();
		auto& info = vulkanImage->GetImageInfo();

		// Image memory barriers for the texture image

		// The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
		VkImageSubresourceRange subresourceRange = {};
		// Image only contains color data
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		// Start at first mip level
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;

		// Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = info.Image;
		imageMemoryBarrier.subresourceRange = subresourceRange;
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition 
		// Source pipeline stage is host write/read exection (VK_PIPELINE_STAGE_HOST_BIT)
		// Destination pipeline stage is copy command exection (VK_PIPELINE_STAGE_TRANSFER_BIT)
		vkCmdPipelineBarrier(
			copyCmd,
			VK_PIPELINE_STAGE_HOST_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier
		);
	}
	void VulkanTexture2D::copyBufferToImage(VkCommandBuffer copyCmd, VkBuffer stagingBuffer)
	{
		Ref<VulkanImage2D> vulkanImage = m_Image.As<VulkanImage2D>();
		auto& info = vulkanImage->GetImageInfo();
		

		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = m_Width;
		bufferCopyRegion.imageExtent.height = m_Height;
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = 0;

		// Copy mip levels from staging buffer
		vkCmdCopyBufferToImage(
			copyCmd,
			stagingBuffer,
			info.Image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&bufferCopyRegion
		);
	}
	void VulkanTexture2D::transitionToDstLayout(VkCommandBuffer copyCmd, uint32_t mipCount) const
	{
		Ref<VulkanImage2D> vulkanImage = m_Image.As<VulkanImage2D>();
		auto& info = vulkanImage->GetImageInfo();

		// The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
		VkImageSubresourceRange subresourceRange = {};
		// Image only contains color data
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		// Start at first mip level
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;

		if (mipCount > 1) // Mips to generate
		{
			VulkanRendererAPI::InsertImageMemoryBarrier(copyCmd, info.Image,
				VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				subresourceRange
			);
		}
		else
		{
			VulkanRendererAPI::InsertImageMemoryBarrier(copyCmd, info.Image,
				VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vulkanImage->GetDescriptor().imageLayout,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				subresourceRange
			);
		}
	}
	bool VulkanTexture2D::loadImage(const std::string& path)
	{
		int width, height, channels;

		if (stbi_is_hdr(path.c_str()))
		{
			m_ImageData.m_Data = (uint8_t*)stbi_loadf(path.c_str(), &width, &height, &channels, 4);
			m_ImageData.m_Size = width * height * 4 * sizeof(float);
			m_Format = ImageFormat::RGBA32F;
		}
		else
		{
			//stbi_set_flip_vertically_on_load(1);
			m_ImageData.m_Data = stbi_load(path.c_str(), &width, &height, &channels, 4);
			m_ImageData.m_Size = width * height * 4;
			m_Format = ImageFormat::RGBA;
		}

		XYZ_ASSERT(m_ImageData.m_Data, "Failed to load image!");
		if (!m_ImageData.m_Data)
			return false;

		m_Width = width;
		m_Height = height;
		return true;
	}

	VmaAllocation VulkanTexture2D::allocateStagingBuffer(VulkanAllocator& allocator, VkBuffer& stagingBuffer) const
	{
		VkDeviceSize size = m_ImageData.m_Size;

		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		return allocator.AllocateBuffer(bufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);
	}

	VkSamplerCreateInfo VulkanTexture2D::createSamplerCreateInfo(uint32_t mipCount) const
	{
		VkSamplerCreateInfo sampler{};
		sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler.maxAnisotropy = 1.0f;
		sampler.magFilter = Utils::VulkanSamplerFilter(m_Properties.SamplerFilter);
		sampler.minFilter = Utils::VulkanSamplerFilter(m_Properties.SamplerFilter);
		sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler.addressModeU = Utils::VulkanSamplerWrap(m_Properties.SamplerWrap);
		sampler.addressModeV = Utils::VulkanSamplerWrap(m_Properties.SamplerWrap);
		sampler.addressModeW = Utils::VulkanSamplerWrap(m_Properties.SamplerWrap);
		sampler.mipLodBias = 0.0f;
		sampler.compareOp = VK_COMPARE_OP_NEVER;
		sampler.minLod = 0.0f;
		sampler.maxLod = (float)mipCount;
		sampler.maxAnisotropy = 1.0;
		sampler.anisotropyEnable = VK_FALSE;
		sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		return sampler;
	}

	VkImageViewCreateInfo VulkanTexture2D::createImageViewCreateInfo(uint32_t layerCount, uint32_t baseLayer, uint32_t mipCount, uint32_t baseMip) const
	{
		VkImageViewCreateInfo view{};
		view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view.format = Utils::VulkanImageFormat(m_Format);
		view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		// The subresource range describes the set of mip levels (and array layers) that can be accessed through this image view
		// It's possible to create multiple image views for a single image referring to different (and/or overlapping) ranges of the image
		view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view.subresourceRange.baseMipLevel = baseMip;
		view.subresourceRange.baseArrayLayer = baseLayer;
		view.subresourceRange.layerCount = layerCount;
		view.subresourceRange.levelCount = mipCount;
		return view;
	}	
}