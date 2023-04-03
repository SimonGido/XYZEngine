#pragma once
#include "XYZ/Asset/Renderer/MaterialAsset.h"

namespace XYZ {


	struct PipelineCachceStatistics
	{
		uint32_t GeometryPipelinesCount = 0;
		uint32_t DepthPipelinesCount = 0;
		uint32_t ComputePipelinesCount = 0;
	};

	class XYZ_API PipelineCache
	{
	public:
		Ref<Pipeline> PreparePipeline(const Ref<MaterialAsset>& materialAsset, const Ref<RenderPass>& renderPass);
		Ref<PipelineCompute> PrepareComputePipeline(const Ref<MaterialAsset>& materialAsset);

		PipelineCachceStatistics GetStatistics() const { return m_Statistics; }
	private:

		struct Pipelines
		{
			Ref<Pipeline> ColorPipeline;
			Ref<Pipeline> DepthPipeline;
		};


		std::map<AssetHandle, Ref<Pipeline>> m_GeometryPipelines;
		std::map<AssetHandle, Ref<Pipeline>> m_DepthPipelines;
		std::map<AssetHandle, Ref<PipelineCompute>> m_ComputePipelines;

		PipelineCachceStatistics m_Statistics;
	};
}