#pragma once
#include <memory>
#include <vector>

#include "XYZ/Core/Ref.h"
#include "XYZ/Scene/Serializable.h"

#include <glm/glm.hpp>

namespace XYZ {
	/*! @class FrameBufferFormat
	*	@brief Describes the color format of the FrameBuffer
	*/
	enum class FrameBufferTextureFormat
	{
		None = 0,

		// Color
		RGBA8 = 1,
		RGBA16F = 2,
		RGBA32F = 3,
		RG32F = 4,
		R32I = 5,

		// Depth/stencil
		DEPTH32F = 6,
		DEPTH24STENCIL8 = 7,

		// Defaults
		Depth = DEPTH24STENCIL8
	};

	struct FrameBufferTextureSpecs
	{
		FrameBufferTextureSpecs() = default;
		FrameBufferTextureSpecs(FrameBufferTextureFormat format) : TextureFormat(format) {}

		FrameBufferTextureFormat TextureFormat;
	};

	struct FrameBufferAttachmentSpecs
	{
		FrameBufferAttachmentSpecs() = default;
		FrameBufferAttachmentSpecs(const std::initializer_list<FrameBufferTextureSpecs>& attachments)
			: Attachments(attachments) {}

		std::vector<FrameBufferTextureSpecs> Attachments;
	};

	struct FrameBufferSpecs
	{
		uint32_t Width = 1280;
		uint32_t Height = 720;
		uint32_t Samples = 1; // multisampling
		glm::vec4 ClearColor;
		FrameBufferAttachmentSpecs Attachments;

		bool SwapChainTarget = false;
	};


	class FrameBuffer : public RefCount,
						public Serializable
	{
	public:
		virtual ~FrameBuffer() = default;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		
		virtual void BindTexture(uint32_t attachmentIndex, uint32_t slot) const = 0;
		virtual void SetSpecification(const FrameBufferSpecs& specs) = 0;

		virtual const uint32_t GetColorAttachmentRendererID(uint32_t index) const = 0;
		virtual const uint32_t GetDetphAttachmentRendererID() const = 0;

		virtual const FrameBufferSpecs& GetSpecification() const = 0;
		
		virtual int32_t ReadPixel(uint32_t mx, uint32_t my, uint32_t attachmentIndex) const = 0;

		static Ref<FrameBuffer> Create(const FrameBufferSpecs& specs);
	};

}