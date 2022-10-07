#include "stdafx.h"
#include "CompositePass.h"


namespace XYZ {
	void CompositePass::Init(const CompositePassConfiguration& config, const Ref<RenderCommandBuffer>& commandBuffer)
	{
		m_RenderPass = config.Pass;
		m_Shader = config.Shader;
		createPipeline();
	}

	void CompositePass::Submit(const Ref<RenderCommandBuffer>& commandBuffer, const Ref<Image2D>& lightImage, const Ref<Image2D>& bloomImage, bool clear)
	{
		XYZ_PROFILE_FUNC("CompositePass::Submit");

		Renderer::BeginRenderPass(commandBuffer, m_RenderPass, clear);
		m_Material->SetImage("u_GeometryTexture", lightImage);
		m_Material->SetImage("u_BloomTexture", bloomImage);
		Renderer::BindPipeline(
			commandBuffer,
			m_Pipeline,
			nullptr,
			nullptr,
			m_Material
		);
		Renderer::SubmitFullscreenQuad(commandBuffer, m_Pipeline, m_MaterialInstance);
		Renderer::EndRenderPass(commandBuffer);
	}

	void CompositePass::createPipeline()
	{
		PipelineSpecification specification;
		specification.Shader = m_Shader;
		specification.RenderPass = m_RenderPass;
		specification.Topology = PrimitiveTopology::Triangles;
		specification.DepthWrite = false;
		m_Pipeline = Pipeline::Create(specification);
		m_Material = Material::Create(m_Shader);
		m_MaterialInstance = Ref<XYZ::MaterialInstance>::Create(m_Material);
	}
}