#include "stdafx.h"
#include "VoxelRenderer.h"

#include "Renderer.h"

#include <glm/gtc/type_ptr.hpp>


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace XYZ {
	VoxelRenderer::VoxelRenderer()
	{
		m_CommandBuffer = PrimaryRenderCommandBuffer::Create(0, "VoxelRenderer");

		const uint32_t framesInFlight = Renderer::GetConfiguration().FramesInFlight;
		m_UniformBufferSet = UniformBufferSet::Create(framesInFlight);
		m_UniformBufferSet->Create(sizeof(UBVoxelScene), UBVoxelScene::Set, UBVoxelScene::Binding);

		m_StorageBufferSet = StorageBufferSet::Create(framesInFlight);
		m_StorageBufferSet->Create(sizeof(SSBOVoxels), SSBOVoxels::Set, SSBOVoxels::Binding);
	
	
		TextureProperties props;
		props.Storage = true;
		props.SamplerWrap = TextureWrap::Clamp;
		m_OutputTexture = Texture2D::Create(ImageFormat::RGBA32F, 1280, 720, nullptr, props);
	
		createRaymarchPipeline();
	}
	void VoxelRenderer::BeginScene(const glm::mat4& viewProjectionMatrix, const glm::mat4& viewMatrix, const glm::mat4& projection, const glm::vec3& cameraPosition)
	{
		m_CurrentVoxelsCount = 0;
		m_UBVoxelScene.InverseProjection = glm::inverse(projection);
		m_UBVoxelScene.InverseView = glm::inverse(viewMatrix);
		m_UBVoxelScene.CameraPosition = glm::vec4(cameraPosition, 0.0f);
		m_UBVoxelScene.LightDirection = { -0.2f, -1.4f, -1.5f, 1.0f };
		m_UBVoxelScene.LightColor = glm::vec4(1.0f);

		m_UBVoxelScene.ViewportSize.x = m_ViewportSize.x;
		m_UBVoxelScene.ViewportSize.y = m_ViewportSize.y;
		m_DrawCommands.clear();

		updateViewportSize();
		updateUniformBufferSet();
	}
	void VoxelRenderer::EndScene()
	{
		updateStorageBufferSet();
		render();
	}
	void VoxelRenderer::SetViewportSize(uint32_t width, uint32_t height)
	{
		if (m_ViewportSize.x != width || m_ViewportSize.y != height)
		{
			m_ViewportSize = glm::ivec2(width, height);
			m_ViewportSizeChanged = true;
		}
	}
	void VoxelRenderer::SubmitVoxels(const VoxelsSpecification& spec, uint32_t* voxels)
	{
		const uint32_t voxelCount = spec.Width * spec.Height * spec.Depth;
		auto& drawCommand = m_DrawCommands.emplace_back();
		drawCommand.Specification = spec;
		drawCommand.VoxelOffset = m_CurrentVoxelsCount;
		drawCommand.VoxelCount = voxelCount;

		memcpy(&m_SSBOVoxels.Voxels[m_CurrentVoxelsCount], voxels, voxelCount * sizeof(uint32_t));

		m_CurrentVoxelsCount += voxelCount;
	}
	void VoxelRenderer::OnImGuiRender()
	{
		if (ImGui::Begin("Voxel Renderer"))
		{
			ImGui::DragFloat3("Light Direction", glm::value_ptr(m_UBVoxelScene.LightDirection), 0.1f);
			ImGui::DragFloat3("Light Color", glm::value_ptr(m_UBVoxelScene.LightColor), 0.1f);
		}
		ImGui::End();
	}

	static glm::mat4 CalcViewMatrix(glm::vec3 pos, float pitch, float yaw)
	{
		const glm::quat orientation = glm::quat(glm::vec3(-pitch, -yaw, 0.0f));
		glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), pos) * glm::toMat4(orientation);
	
		viewMatrix = glm::inverse(viewMatrix);
		return viewMatrix;
	}

	void VoxelRenderer::render()
	{
		m_CommandBuffer->Begin();
		Renderer::BeginPipelineCompute(
			m_CommandBuffer,
			m_RaymarchPipeline,
			m_UniformBufferSet,
			m_StorageBufferSet,
			m_RaymarchMaterial
		);

		for (const auto& drawCommand : m_DrawCommands)
		{
			m_StorageBufferSet->SetBufferInfo(
				drawCommand.VoxelCount * sizeof(uint32_t),
				drawCommand.VoxelOffset * sizeof(uint32_t),
				SSBOVoxels::Binding,
				SSBOVoxels::Set
			);
			Renderer::DispatchCompute(
				m_RaymarchPipeline,
				nullptr,
				32, 32, 1,
				PushConstBuffer{ drawCommand.Specification }
			);
		}
		Renderer::EndPipelineCompute(m_RaymarchPipeline);

		m_CommandBuffer->End();
		m_CommandBuffer->Submit();
	}
	void VoxelRenderer::updateViewportSize()
	{
		if (m_ViewportSizeChanged)
		{
			m_ViewportSizeChanged = false;
			TextureProperties props;
			props.Storage = true;
			props.SamplerWrap = TextureWrap::Clamp;
			m_OutputTexture = Texture2D::Create(ImageFormat::RGBA32F, m_ViewportSize.x, m_ViewportSize.y, nullptr, props);
			m_RaymarchMaterial->SetImage("o_Image", m_OutputTexture->GetImage());
		}
	}
	void VoxelRenderer::updateUniformBufferSet()
	{
		Ref<VoxelRenderer> instance = this;
		Renderer::Submit([instance]() mutable {

			const uint32_t currentFrame = Renderer::GetCurrentFrame();
			instance->m_UniformBufferSet->Get(UBVoxelScene::Binding, UBVoxelScene::Set, currentFrame)->RT_Update(&instance->m_UBVoxelScene, sizeof(UBVoxelScene), 0);

		});
	}
	void VoxelRenderer::updateStorageBufferSet()
	{
		Ref<VoxelRenderer> instance = this;
		uint32_t voxelCount = m_CurrentVoxelsCount;
		Renderer::Submit([instance, voxelCount]() mutable {
			const uint32_t currentFrame = Renderer::GetCurrentFrame();
			instance->m_StorageBufferSet->Get(SSBOVoxels::Binding, SSBOVoxels::Set, currentFrame)->RT_Update(&instance->m_SSBOVoxels, voxelCount * sizeof(uint32_t), 0);
		});
	}
	void VoxelRenderer::createRaymarchPipeline()
	{
		Ref<Shader> shader = Shader::Create("Resources/Shaders/RaymarchShader.glsl");
		m_RaymarchMaterial = Material::Create(shader);

		m_RaymarchMaterial->SetImage("o_Image", m_OutputTexture->GetImage());
		PipelineComputeSpecification spec;
		spec.Shader = shader;
	
		m_RaymarchPipeline = PipelineCompute::Create(spec);
	}
}
