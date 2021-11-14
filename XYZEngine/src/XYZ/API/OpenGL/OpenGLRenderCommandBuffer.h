#pragma once
#include "XYZ/Renderer/RenderCommandBuffer.h"

namespace XYZ {
	class OpenGLRenderCommandBuffer : public RenderCommandBuffer
	{
	public:
		OpenGLRenderCommandBuffer(uint32_t count, std::string debugName) {}
		
		virtual void Begin() override { };
		virtual void End() override {};
		virtual void Submit() override {};

		virtual float GetExecutionGPUTime(uint32_t frameIndex, uint32_t queryIndex = 0) const override { return 0.0f; }

		virtual uint64_t BeginTimestampQuery() override { return 0; }
		virtual void EndTimestampQuery(uint64_t queryID) override {};
	};
}