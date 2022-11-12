#pragma once
#include "XYZ/Core/Ref/Ref.h"
#include "Shader.h"
#include "RenderCommandBuffer.h"
#include "PipelineSpecialization.h"

namespace XYZ {


	struct PipelineComputeSpecification
	{
		Ref<Shader> Shader;
		PipelineSpecialization Specialization;
	};

	class XYZ_API PipelineCompute : public RefCount
	{
	public:
		virtual ~PipelineCompute() = default;
		
		virtual void Begin(Ref<RenderCommandBuffer> renderCommandBuffer = nullptr) = 0;
		virtual void End() = 0;
		virtual void Invalidate() = 0;

		virtual Ref<Shader> GetShader() const = 0;

		static Ref<PipelineCompute> Create(const PipelineComputeSpecification& specification);
	};
	
}