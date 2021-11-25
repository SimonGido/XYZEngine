#pragma once
#include <memory>
#include <vector>

#include "XYZ/Core/Ref.h"
#include "XYZ/Renderer/Texture.h"
#include "XYZ/Renderer/Image.h"

#include <glm/glm.hpp>

namespace XYZ {
	enum class FramebufferBlendMode
	{
		None = 0,
		OneZero,
		SrcAlphaOneMinusSrcAlpha,
		Additive,
		Zero_SrcColor
	};
	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(ImageFormat format, bool generateMips = false)
			: Format(format), GenerateMips(generateMips) {}

		ImageFormat			 Format;
		bool				 GenerateMips;
		bool				 Blend = true;
		FramebufferBlendMode BlendMode = FramebufferBlendMode::SrcAlphaOneMinusSrcAlpha;
	};


	struct FramebufferSpecification
	{
		uint32_t  Width = 0;
		uint32_t  Height = 0;
		uint32_t  Samples = 1; // multisampling
		glm::vec4 ClearColor  = { 0.0f, 0.0f, 0.0f, 1.0f };
		bool	  ClearOnLoad = true;
		std::vector<FramebufferTextureSpecification> Attachments;

		bool SwapChainTarget = false;
	};


	class Framebuffer : public RefCount
	{
	public:
		virtual ~Framebuffer() = default;

		virtual void Resize(uint32_t width, uint32_t height, bool forceResize = false) {};

		virtual void Bind() const {};
		virtual void Unbind() const {};
		virtual void Clear() const {};

		virtual void BindTexture(uint32_t attachmentIndex, uint32_t slot) const {};
		virtual void BindImage(uint32_t attachmentIndex, uint32_t slot, uint32_t miplevel, BindImageType type) const {};
		virtual void SetSpecification(const FramebufferSpecification& specs) {};

		virtual Ref<Image2D> GetImage(uint32_t attachmentIndex = 0) const { return Ref<Image2D>(); };
		virtual Ref<Image2D> GetDepthImage() const { return Ref<Image2D>(); }
		virtual const uint32_t GetNumColorAttachments() const { return 0; };

		virtual const FramebufferSpecification& GetSpecification() const = 0;
		
		virtual void ReadPixel(int32_t& pixel, uint32_t mx, uint32_t my, uint32_t attachmentIndex) const {};
		virtual void ClearColorAttachment(uint32_t colorAttachmentIndex, void* clearValue) const {};

		static Ref<Framebuffer> Create(const FramebufferSpecification& specs);
	};

}