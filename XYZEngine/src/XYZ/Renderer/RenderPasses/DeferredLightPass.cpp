#include "stdafx.h"
#include "DeferredLightPass.h"
#include "XYZ/Utils/Math/Math.h"

#include "XYZ/Scene/Components.h"

#include "XYZ/Debug/Profiler.h"

namespace XYZ {

	void DeferredLightPass::Init(const DeferredLightPassConfiguration& config, const Ref<RenderCommandBuffer>& commandBuffer)
	{
		m_RenderPass = config.Pass;
		m_Shader = config.Shader;
		createPipeline();

		m_LightStorageBufferSet = StorageBufferSet::Create(Renderer::GetConfiguration().FramesInFlight);
		
		constexpr uint32_t countOffset = 16;
		m_LightStorageBufferSet->Create(countOffset + sc_MaxNumberOfLights * sizeof(PointLight2D), 0, 1);
		m_LightStorageBufferSet->Create(countOffset + sc_MaxNumberOfLights * sizeof(SpotLight2D), 0, 2);
	}

	DeferredLightPassStatistics DeferredLightPass::PreSubmit(Ref<Scene> scene)
	{
		XYZ_PROFILE_FUNC("DeferredLightPass::PreSubmit");
		
		const auto& lightEnvironment = scene->GetLightEnvironment();

		constexpr uint32_t countOffset = 16;

		const uint32_t spotLightsCount = std::min(static_cast<uint32_t>(lightEnvironment.SpotLights2D.size()), sc_MaxNumberOfLights);
		const uint32_t pointLightsCount = std::min(static_cast<uint32_t>(lightEnvironment.PointLights2D.size()), sc_MaxNumberOfLights);


		ByteBuffer spotLightBuffer;
		spotLightBuffer.Allocate(countOffset + (spotLightsCount * sizeof(SpotLight2D)));
		spotLightBuffer.Write(spotLightsCount, 0);
		spotLightBuffer.Write(lightEnvironment.SpotLights2D.data(), spotLightsCount * sizeof(SpotLight2D), countOffset);

		ByteBuffer pointLightBuffer;
		pointLightBuffer.Allocate(countOffset + (pointLightsCount * sizeof(PointLight2D)));
		pointLightBuffer.Write(pointLightsCount, 0);
		pointLightBuffer.Write(lightEnvironment.PointLights2D.data(), pointLightsCount * sizeof(PointLight2D), countOffset);

		Ref<StorageBufferSet> instance = m_LightStorageBufferSet;
		Renderer::Submit([
			instance,
				pointLightBuffer,
				spotLightBuffer
		]() mutable
			{
				const uint32_t frame = Renderer::GetCurrentFrame();
				instance->Get(1, 0, frame)->RT_Update(pointLightBuffer.Data, pointLightBuffer.Size);
				instance->Get(2, 0, frame)->RT_Update(spotLightBuffer.Data, spotLightBuffer.Size);

				// TODO: recycle
				pointLightBuffer.Destroy();
				spotLightBuffer.Destroy();
			});

		return { spotLightsCount, pointLightsCount };
	}

	void DeferredLightPass::Submit(
		const Ref<RenderCommandBuffer>& commandBuffer,
		const Ref<Image2D>& colorImage,
		const Ref<Image2D>& positionImage
	)
	{
		XYZ_PROFILE_FUNC("DeferredLightPass::Submit");
		Renderer::BeginRenderPass(commandBuffer, m_RenderPass, false, true);
	
		m_Material->SetImageArray("u_Texture", colorImage, 0);
		m_Material->SetImageArray("u_Texture", positionImage, 1);


		Renderer::BindPipeline(
			commandBuffer,
			m_Pipeline,
			m_CameraBufferSet,
			m_LightStorageBufferSet,
			m_Material
		);

		Renderer::SubmitFullscreenQuad(commandBuffer, m_Pipeline, m_MaterialInstance);
		Renderer::EndRenderPass(commandBuffer);
	}

	void DeferredLightPass::createPipeline()
	{
		PipelineSpecification specification;
		specification.Shader = m_Shader;
		specification.RenderPass = m_RenderPass;
		specification.Topology = PrimitiveTopology::Triangles;
		specification.DepthWrite = false;
		m_Pipeline = Pipeline::Create(specification);
		m_Material = Material::Create(m_Shader);
		m_MaterialInstance = m_Material->CreateMaterialInstance();
	}
}
