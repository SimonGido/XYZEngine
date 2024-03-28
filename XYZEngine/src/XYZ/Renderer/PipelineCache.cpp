#include "stdafx.h"
#include "PipelineCache.h"


namespace XYZ {
	Ref<Pipeline> PipelineCache::PreparePipeline(const Ref<MaterialAsset>& materialAsset, const Ref<RenderPass>& renderPass)
	{		
		auto it = m_GeometryPipelines.find(materialAsset->GetHandle());
		if (it != m_GeometryPipelines.end())
			return it->second;

		m_Statistics.GeometryPipelinesCount++;

		PipelineSpecification spec;
		spec.RenderPass = renderPass;
		spec.Shader = materialAsset->GetShader();
		spec.Topology = PrimitiveTopology::Triangles;
		spec.DepthTest = true;
		spec.DepthWrite = true;

		auto& pipeline = m_GeometryPipelines[materialAsset->GetHandle()];
		pipeline = Pipeline::Create(spec);
		return pipeline;
	}

	Ref<PipelineCompute> PipelineCache::PrepareComputePipeline(const Ref<MaterialAsset>& materialAsset)
	{
		auto it = m_ComputePipelines.find(materialAsset->GetHandle());
		if (it != m_ComputePipelines.end() && !materialAsset->GetSpecialization().m_Dirty)
			return it->second;

		m_Statistics.ComputePipelinesCount += it != m_ComputePipelines.end() ? 1 : 0;

		materialAsset->GetSpecialization().m_Dirty = false;

		PipelineComputeSpecification specification;
		specification.Shader = materialAsset->GetShader();
		specification.Specialization = materialAsset->GetSpecialization();

		auto& pipeline = m_ComputePipelines[materialAsset->GetHandle()];
		pipeline = PipelineCompute::Create(specification);
		return pipeline;
	}
}