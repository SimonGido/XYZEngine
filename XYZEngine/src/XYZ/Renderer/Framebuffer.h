#pragma once
#include <memory>
#include <vector>

#include "XYZ/Core/Ref.h"
#include "XYZ/Renderer/Texture.h"
#include "XYZ/Renderer/Image.h"

#include <glm/glm.hpp>

namespace XYZ {

	struct FramebufferTextureSpecs
	{
		FramebufferTextureSpecs() = default;
		FramebufferTextureSpecs(ImageFormat format, bool generateMips = false) 
			: TextureFormat(format), GenerateMips(generateMips){}

		ImageFormat TextureFormat;
		bool		GenerateMips;
	};

	struct FramebufferAttachmentSpecs
	{
		FramebufferAttachmentSpecs() = default;
		FramebufferAttachmentSpecs(const std::initializer_list<FramebufferTextureSpecs>& attachments)
			: Attachments(attachments) {}

		std::vector<FramebufferTextureSpecs> Attachments;
	};
	

	struct FramebufferSpecs
	{
		uint32_t Width = 1280;
		uint32_t Height = 720;
		uint32_t Samples = 1; // multisampling
		glm::vec4 ClearColor;
		FramebufferAttachmentSpecs Attachments;

		bool SwapChainTarget = false;
	};


	class Framebuffer : public RefCount
	{
	public:
		virtual ~Framebuffer() = default;

		virtual void Resize(uint32_t width, uint32_t height, bool forceResize = false) = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void Clear() const = 0;

		virtual void BindTexture(uint32_t attachmentIndex, uint32_t slot) const = 0;
		virtual void BindImage(uint32_t attachmentIndex, uint32_t slot, uint32_t miplevel, BindImageType type) const = 0;
		virtual void SetSpecification(const FramebufferSpecs& specs) = 0;

		virtual const uint32_t GetColorAttachmentRendererID(uint32_t index) const = 0;
		virtual const uint32_t GetDetphAttachmentRendererID() const = 0;
		virtual const uint32_t GetNumColorAttachments() const = 0;

		virtual const FramebufferSpecs& GetSpecification() const = 0;
		
		virtual void ReadPixel(int32_t& pixel, uint32_t mx, uint32_t my, uint32_t attachmentIndex) const = 0;
		virtual void ClearColorAttachment(uint32_t colorAttachmentIndex, void* clearValue) const = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecs& specs);
	};

}