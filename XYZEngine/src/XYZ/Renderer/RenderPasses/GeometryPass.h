#pragma once
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Debug/Profiler.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/GeometryRenderQueue.h"

#include "XYZ/Renderer/SceneRendererBuffers.h"

#include "XYZ/Renderer/PipelineCache.h"

namespace XYZ {

	struct GeometryPassStatistics
	{
		uint32_t MeshOverrideCount;
		uint32_t AnimatedMeshOverrideCount;
		uint32_t TransformInstanceCount;
		uint32_t InstanceDataSize;
		uint32_t BoneTransformCount;
	};

	class XYZ_API GeometryPass
	{
	public:
		GeometryPass();

		void Init(WeakRef<SceneRenderer> renderer);

		void PreDepthPass(
			Ref<PrimaryRenderCommandBuffer> commandBuffer,
			GeometryRenderQueue& queue,
			const glm::mat4& viewMatrix,
			bool clear
		);
		void SubmitCompute(
			Ref<PrimaryRenderCommandBuffer> commandBuffer,
			GeometryRenderQueue& queue);

		void Submit(
			Ref<PrimaryRenderCommandBuffer> commandBuffer,
			GeometryRenderQueue& queue, 
			const glm::mat4& viewMatrix,
			bool clear
		);
		GeometryPassStatistics PreSubmit(GeometryRenderQueue& queue);


		static constexpr uint32_t GetInstanceBufferSize() { return sc_InstanceVertexBufferSize; }
		static constexpr uint32_t GetTransformBufferSize() { return sc_TransformBufferSize; }
		static constexpr uint32_t GetTransformBufferCount() { return sc_TransformBufferSize / sizeof(GeometryRenderQueue::TransformData); }
	private:
		void submitComputeCommands(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer);
		void submitIndirectCommands(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer);
		void submitStaticMeshes(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer);
		void submitAnimatedMeshes(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer);
		void submitInstancedMeshes(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer);
		void submit2D(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer, const glm::mat4& viewMatrix);

		void submitStaticMeshesDepth(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer);
		void submitAnimatedMeshesDepth(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer);
		void submitInstancedMeshesDepth(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer);
		void submit2DDepth(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer, const glm::mat4& viewMatrix);
		void postDepthPass(const Ref<RenderCommandBuffer>& commandBuffer);


		void prepareComputeCommands(GeometryRenderQueue& queue);
		void prepareIndirectCommands(GeometryRenderQueue& queue);
		void prepareStaticDrawCommands(GeometryRenderQueue& queue, size_t& overrideCount, uint32_t& transformsCount);
		void prepareAnimatedDrawCommands(GeometryRenderQueue& queue, size_t& overrideCount, uint32_t& transformsCount, uint32_t& boneTransformCount);
		void prepareInstancedDrawCommands(GeometryRenderQueue& queue, uint32_t& instanceOffset);
		void prepare2DDrawCommands(GeometryRenderQueue& queue);

		void createDepthResources();
		
	private:	
		WeakRef<SceneRenderer> m_SceneRenderer;
		Ref<Renderer2D>		   m_Renderer2D;
		Ref<Texture2D>		   m_WhiteTexture;

		Ref<VertexBufferSet>   m_InstanceVertexBufferSet;
		Ref<VertexBufferSet>   m_TransformVertexBufferSet;

		PipelineCache m_PipelineCache;

		struct DepthPipeline
		{
			Ref<Pipeline>			Pipeline;
			Ref<Shader>				Shader;
			Ref<Material>			Material;
			Ref<MaterialInstance>	MaterialInstance;
		};

		DepthPipeline m_DepthPipeline3DStatic;
		DepthPipeline m_DepthPipeline3DAnimated;
		DepthPipeline m_DepthPipeline2D;
		DepthPipeline m_DepthPipelineInstanced;


		

		static constexpr uint32_t sc_TransformBufferSize	  = 10 * 1024 * sizeof(GeometryRenderQueue::TransformData); // 10240 transforms
		static constexpr uint32_t sc_InstanceVertexBufferSize = 30 * 1024 * 1024; // 30mb
	};
}