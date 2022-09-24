#pragma once
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Debug/Profiler.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/GeometryRenderQueue.h"

namespace XYZ {

	struct GeometryPassConfiguration
	{
		Ref<RenderPass>		  Pass;
		Ref<UniformBufferSet> CameraBufferSet;
	};

	struct GeometryPassStatistics
	{
		uint32_t MeshOverrideCount;
		uint32_t TransformInstanceCount;
		uint32_t InstanceDataSize;
	};

	class GeometryPass3D
	{
	public:
		GeometryPass3D();

		void Init(const GeometryPassConfiguration& config);

		void Submit(const Ref<RenderCommandBuffer>& commandBuffer, GeometryRenderQueue& queue, bool clear);
		GeometryPassStatistics PreSubmit(GeometryRenderQueue& queue);


		static constexpr uint32_t GetMaxBonesTransforms() { return sc_MaxBoneTransforms; }
		static constexpr uint32_t GetInstanceBufferSize() { return sc_InstanceVertexBufferSize; }
		static constexpr uint32_t GetTransformBufferSize() { return sc_TransformBufferSize; }
		static constexpr uint32_t GetTransformBufferCount() { return sc_TransformBufferSize / sizeof(GeometryRenderQueue::TransformData); }
	private:
		void submitStaticMeshes(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer);
		void submitAnimatedMeshes(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer);
		void submitInstancedMeshes(GeometryRenderQueue& queue, const Ref<RenderCommandBuffer>& commandBuffer);

		void prepareStaticDrawCommands(GeometryRenderQueue& queue, size_t& overrideCount, uint32_t& transformsCount);
		void prepareAnimatedDrawCommands(GeometryRenderQueue& queue, size_t& overrideCount, uint32_t& transformsCount, uint32_t& boneTransformCount);
		void prepareInstancedDrawCommands(GeometryRenderQueue& queue, uint32_t& instanceOffset);

		Ref<Pipeline> prepareGeometryPipeline(const Ref<Material>& material, bool opaque);
	private:
		using TransformData = GeometryRenderQueue::TransformData;

		Ref<RenderPass>       m_RenderPass;
		Ref<UniformBufferSet> m_CameraBufferSet;

		Ref<VertexBufferSet>  m_InstanceVertexBufferSet;
		Ref<VertexBufferSet>  m_TransformVertexBufferSet;
		Ref<StorageBufferSet> m_BoneTransformsStorageSet;

		std::map<size_t, Ref<Pipeline>> m_GeometryPipelines;

		std::vector<TransformData> m_TransformData;
		std::vector<glm::mat4>	   m_BoneTransformsData;
		std::vector<std::byte>	   m_InstanceData;


		static constexpr uint32_t sc_TransformBufferSize	  = 10 * 1024 * sizeof(GeometryRenderQueue::TransformData); // 10240 transforms
		static constexpr uint32_t sc_InstanceVertexBufferSize = 30 * 1024 * 1024; // 30mb
		static constexpr uint32_t sc_MaxBoneTransforms		  = 1 * 1024;

	};


	class GeometryPass2D
	{
	public:
		GeometryPass2D();

		void Init(const GeometryPassConfiguration& config, const Ref<RenderCommandBuffer>& commandBuffer);

		void Submit(
			const Ref<RenderCommandBuffer>& commandBuffer, 
			GeometryRenderQueue& queue, 
			const glm::mat4& viewMatrix,
			bool clear
		);


	private:
		Ref<Renderer2D>		  m_Renderer2D;
		Ref<RenderPass>       m_RenderPass;
		Ref<UniformBufferSet> m_CameraBufferSet;
		Ref<Texture2D>		  m_WhiteTexture;
	};
}