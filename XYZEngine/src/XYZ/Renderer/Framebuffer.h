#pragma once
#include <memory>

#include "XYZ/Core/Ref.h"

#include <glm/glm.hpp>

namespace XYZ {
	/*! @class FrameBufferFormat
	*	@brief Describes the color format of the FrameBuffer
	*/
	enum class FrameBufferFormat
	{
		None = 0,
		RGB = 1,
		RGBA8 = 2,
		RGBA16F = 3
	};

	struct ColorAttachment
	{
		uint32_t RendererID;
		FrameBufferFormat Format;
	};

	struct DepthAttachment
	{
		uint32_t RendererID;
	};

	struct FrameBufferSpecs
	{
		uint32_t Width;
		uint32_t Height;
		glm::vec4 ClearColor;
		uint32_t Samples = 1;
		FrameBufferFormat Format = FrameBufferFormat::RGBA16F;
		bool SwapChainTarget = false; 
	};

	/*! @class FrameBuffer
	*	@brief Framebuffer for texture operations
	*/
	class FrameBuffer : public RefCount
	{
	public:
		virtual ~FrameBuffer() = default;

		virtual void Resize() = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetSpecification(const FrameBufferSpecs& specs) = 0;

		virtual void CreateColorAttachment(FrameBufferFormat format) = 0;
		virtual void CreateDepthAttachment() = 0;

		virtual ColorAttachment GetColorAttachment(uint32_t index) const = 0;
		virtual DepthAttachment GetDetphAttachment(uint32_t index) const = 0;

		virtual size_t GetNumberColorAttachments() const = 0;
		virtual size_t GetNumberDepthAttachments() const = 0;

		virtual const FrameBufferSpecs& GetSpecification() const = 0;
		virtual FrameBufferSpecs& GetSpecification() = 0;
		
		static Ref<FrameBuffer> Create(const FrameBufferSpecs& specs);
	};

}