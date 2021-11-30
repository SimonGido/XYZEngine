#pragma once

#include "XYZ/Core/Ref.h"

#include "XYZ/Renderer/Buffer.h"
#include "XYZ/Renderer/Shader.h"
#include "XYZ/Renderer/RenderPass.h"

namespace XYZ {

	enum class PrimitiveTopology
	{
		None = 0,
		Points,
		Lines,
		Triangles,
		LineStrip,
		TriangleStrip,
		TriangleFan
	};

	struct PipelineSpecification
	{
		Ref<Shader>		  Shader;
		BufferLayout	  Layout;
		Ref<RenderPass>   RenderPass;
		PrimitiveTopology Topology = PrimitiveTopology::Triangles;
		bool			  BackfaceCulling = true;
		bool			  DepthTest = true;
		bool			  DepthWrite = true;
		bool			  Wireframe = false;
		float			  LineWidth = 1.0f;
		std::string		  DebugName;
	};

	struct PipelineStatistics
	{
		uint64_t InputAssemblyVertices = 0;
		uint64_t InputAssemblyPrimitives = 0;
		uint64_t VertexShaderInvocations = 0;
		uint64_t ClippingInvocations = 0;
		uint64_t ClippingPrimitives = 0;
		uint64_t FragmentShaderInvocations = 0;
		uint64_t ComputeShaderInvocations = 0;

		static constexpr uint32_t Count() { return sizeof(PipelineStatistics) / sizeof(uint64_t); }
	};

	class Pipeline : public RefCount
	{
	public:
		virtual ~Pipeline() = default;

		virtual PipelineSpecification& GetSpecification() = 0;
		virtual const PipelineSpecification& GetSpecification() const = 0;

		virtual void Invalidate() = 0;
		virtual void SetUniformBuffer(Ref<UniformBuffer> uniformBuffer, uint32_t binding, uint32_t set = 0) = 0;


		static Ref<Pipeline> Create(const PipelineSpecification& spec);
	};

}
