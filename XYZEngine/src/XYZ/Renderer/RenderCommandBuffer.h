#pragma once
#include "XYZ/Core/Ref/Ref.h"
#include "Pipeline.h"

namespace XYZ {
	class RenderCommandBuffer : public RefCount
	{
	public:
		virtual ~RenderCommandBuffer() {}

		virtual void Begin() = 0;
		virtual void End() = 0;
		
		virtual void RT_Begin(){};
		virtual void RT_End() {}

		virtual void* CommandBufferHandle(uint32_t index) = 0;
	};


	class PrimaryRenderCommandBuffer : public RenderCommandBuffer
	{
	public:
		virtual ~PrimaryRenderCommandBuffer() {}
		
		virtual void Submit() = 0;

		virtual void CreateTimestampQueries(uint32_t count) = 0;

		virtual float GetExecutionGPUTime(uint32_t frameIndex, uint32_t queryIndex = 0) const = 0;
		virtual const PipelineStatistics& GetPipelineStatistics(uint32_t frameIndex) const = 0;

		virtual uint32_t BeginTimestampQuery() = 0;
		virtual void EndTimestampQuery(uint32_t queryID) = 0;

		virtual Ref<RenderCommandBuffer> CreateSecondaryCommandBuffer() = 0;

		static Ref<PrimaryRenderCommandBuffer> Create(uint32_t count = 0, const std::string& debugName = "");
	};
}