#pragma once
#include "XYZ/Core/Ref.h"
#include "FrameBuffer.h"

namespace XYZ {

	struct RenderPassSpecification
	{
		Ref<FrameBuffer> TargetFramebuffer;
	};

	class RenderPass : public RefCount
	{
	public:
		virtual ~RenderPass() = default;

		virtual RenderPassSpecification& GetSpecification() = 0;
		virtual const RenderPassSpecification& GetSpecification() const = 0;

		static Ref<RenderPass> Create(const RenderPassSpecification & spec);

	};
}