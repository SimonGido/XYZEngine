#pragma once
#include "XYZ/Renderer/Texture.h"
#include "VulkanImage.h"
#include "Vulkan.h"
#include "VulkanAllocator.h"

namespace XYZ {

	class VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D(const std::string& path, const TextureProperties& properties);
		VulkanTexture2D(ImageFormat format, uint32_t width, uint32_t height, const void* data, const TextureProperties& properties);
		~VulkanTexture2D() override;

		
		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual void Lock() override;
		virtual void Unlock() override;
		
		virtual bool			   Loaded() const override { return m_Image->GetBuffer(); }
		virtual uint32_t		   GetMipLevelCount() const override;
		virtual ByteBuffer		   GetWriteableBuffer() override;
		virtual const std::string& GetPath() const override { return m_Path; };
		
		virtual ImageFormat		   GetFormat() const override { return m_Format; }
		virtual uint32_t		   GetWidth() const override { return m_Width; }
		virtual uint32_t		   GetHeight() const override { return m_Height; }

		virtual Ref<Image2D>				  GetImage() const override { return m_Image; }
		virtual const TextureProperties&	  GetProperties() const override { return m_Properties; }
		virtual std::pair<uint32_t, uint32_t> GetMipSize(uint32_t mip) const override;
	
		void Invalidate();
		void RT_Invalidate();
		void GenerateMips();
	
		const VkDescriptorImageInfo& GetVulkanDescriptorInfo() const { return m_Image.As<VulkanImage2D>()->GetDescriptor(); }
	private:
		void loadImage(const std::string& path, ByteBuffer& imageData);

	private:
		std::string		  m_Path;
		uint32_t		  m_Width;
		uint32_t		  m_Height;
		TextureProperties m_Properties;
		Ref<Image2D>	  m_Image;
		ImageFormat		  m_Format = ImageFormat::None;
	};

}