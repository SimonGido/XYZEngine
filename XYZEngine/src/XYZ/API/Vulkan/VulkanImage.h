#pragma once

#include "XYZ/Renderer/Image.h"

#include "Vulkan.h"
#include "VulkanContext.h"
#include "VulkanMemoryAllocator/vk_mem_alloc.h"

namespace XYZ {

	struct VulkanImageInfo
	{
		VkImage Image			  = VK_NULL_HANDLE;
		VkImageView ImageView	  = VK_NULL_HANDLE;
		VkSampler Sampler		  = VK_NULL_HANDLE;
		VmaAllocation MemoryAlloc = VK_NULL_HANDLE;
	};

	class VulkanImage2D : public Image2D
	{
	public:
		VulkanImage2D(const ImageSpecification& specification);
		virtual ~VulkanImage2D() override;

		virtual void Invalidate() override;
		virtual void Release() override;
		virtual void CreatePerLayerImageViews() override;

		virtual uint32_t GetWidth() const override { return m_Specification.Width; }
		virtual uint32_t GetHeight() const override { return m_Specification.Height; }
		virtual float	 GetAspectRatio() const override { return (float)m_Specification.Width / (float)m_Specification.Height; }

		virtual VkImageView GetLayerImageView(uint32_t layer);
		virtual ImageSpecification& GetSpecification() override { return m_Specification; }
		virtual const ImageSpecification& GetSpecification() const override { return m_Specification; }

		virtual ByteBuffer  GetBuffer() const override { return m_ImageData; }
		virtual ByteBuffer& GetBuffer() override	   { return m_ImageData; }
		virtual uint64_t    GetHash() const override   { return (uint64_t)m_Info.Image; }


		void    UpdateDescriptor();
		void	RT_Invalidate(bool createDefault = true);
		void	RT_CreatePerLayerImageViews();
		void	RT_CreatePerSpecificLayerImageViews(const std::vector<uint32_t>& layerIndices);

		

		VkImageView GetMipImageView(uint32_t mip);
		VkImageView RT_GetMipImageView(uint32_t mip);

		VulkanImageInfo& GetImageInfo() { return m_Info; }
		const VulkanImageInfo& GetImageInfo() const { return m_Info; }
		const VkDescriptorImageInfo& GetDescriptor() const { return m_DescriptorImageInfo; }

	private:
		VkImageUsageFlags	  findUsage() const;
		VkImageCreateInfo	  createImageCreateInfo() const;
		VkImageViewCreateInfo createImageViewCreateInfo(uint32_t layerCount, uint32_t baseLayer, uint32_t mipCount, uint32_t baseMip) const;
		VkSamplerCreateInfo   createSamplerCreateInfo() const;

	private:
		ImageSpecification m_Specification;
		VulkanImageInfo	   m_Info;
		ByteBuffer		   m_ImageData;

		std::vector<VkImageView> m_PerLayerImageViews;

		struct MipView
		{
			VkImageView	View = VK_NULL_HANDLE;
			bool		Created = false;
		};
		std::vector<MipView>  m_MipImageViews;
		VkDescriptorImageInfo m_DescriptorImageInfo = {};
	};

	namespace Utils {

		inline VkFormat VulkanImageFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RED32F:          return VK_FORMAT_R32_SFLOAT;
			case ImageFormat::RG16F:		   return VK_FORMAT_R16G16_SFLOAT;
			case ImageFormat::RG32F:		   return VK_FORMAT_R32G32_SFLOAT;
			case ImageFormat::RGBA:            return VK_FORMAT_R8G8B8A8_UNORM;
			case ImageFormat::RGBA16F:         return VK_FORMAT_R16G16B16A16_SFLOAT;
			case ImageFormat::RGBA32F:         return VK_FORMAT_R32G32B32A32_SFLOAT;
			case ImageFormat::DEPTH32F:        return VK_FORMAT_D32_SFLOAT;
			case ImageFormat::DEPTH24STENCIL8: return VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetDepthFormat();
			}
			XYZ_ASSERT(false, "");
			return VK_FORMAT_UNDEFINED;
		}

	}
}