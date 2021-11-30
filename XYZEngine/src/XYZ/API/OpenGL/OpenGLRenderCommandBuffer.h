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
		
		virtual void CreateTimestampQueries(uint32_t count) override {};

		virtual float GetExecutionGPUTime(uint32_t frameIndex, uint32_t queryIndex = 0) const override { return 0.0f; }
		virtual const PipelineStatistics& GetPipelineStatistics(uint32_t frameIndex) const override { return PipelineStatistics(); }

		virtual uint32_t BeginTimestampQuery() override { return 0; }
		virtual void EndTimestampQuery(uint32_t queryID) override {};
	};
}