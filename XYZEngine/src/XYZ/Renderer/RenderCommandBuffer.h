#pragma once
#include "XYZ/Core/Ref.h"

namespace XYZ {
	class RenderCommandBuffer : public RefCount
	{
	public:
		virtual ~RenderCommandBuffer() {}

		virtual void Begin() = 0;
		virtual void End() = 0;
		virtual void Submit() = 0;

		virtual void RT_Begin(){};
		virtual void RT_End() {}
		
		virtual float GetExecutionGPUTime(uint32_t frameIndex, uint32_t queryIndex = 0) const = 0;

		virtual uint64_t BeginTimestampQuery() = 0;
		virtual void EndTimestampQuery(uint64_t queryID) = 0;

		static Ref<RenderCommandBuffer> Create(uint32_t count = 0, const std::string& debugName = "");
	};
}