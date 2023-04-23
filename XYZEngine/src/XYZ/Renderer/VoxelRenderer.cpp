#include "stdafx.h"
#include "VoxelRenderer.h"

#include "Renderer.h"

#include "XYZ/API/Vulkan/VulkanPipelineCompute.h"

#include <glm/gtc/type_ptr.hpp>


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace XYZ {
	static std::random_device s_RandomDev; // obtain a random number from hardware
	static std::mt19937 s_RandomGen(s_RandomDev());

	static uint32_t RandomColor()
	{
		// seed the generator
		std::uniform_int_distribution<> distr(0, 255); // define the range

		uint32_t result = 0;
		result |= static_cast<uint8_t>(distr(s_RandomGen));		  // R
		result |= static_cast<uint8_t>(distr(s_RandomGen)) << 8;  // G
		result |= static_cast<uint8_t>(distr(s_RandomGen)) << 16; // B
		result |= static_cast<uint8_t>(distr(s_RandomGen)) << 24; // A

		return result;
	}



	VoxelRenderer::VoxelRenderer()
	{
		m_CommandBuffer = PrimaryRenderCommandBuffer::Create(0, "VoxelRenderer");
		m_CommandBuffer->CreateTimestampQueries(GPUTimeQueries::Count());

		const uint32_t framesInFlight = Renderer::GetConfiguration().FramesInFlight;
		m_UniformBufferSet = UniformBufferSet::Create(framesInFlight);
		m_UniformBufferSet->Create(sizeof(UBVoxelScene), UBVoxelScene::Set, UBVoxelScene::Binding);
		uint32_t size = sizeof(UBVoxelScene);
		m_StorageBufferSet = StorageBufferSet::Create(framesInFlight);
		m_StorageBufferSet->Create(sizeof(SSBOVoxels), SSBOVoxels::Set, SSBOVoxels::Binding);
		m_StorageBufferSet->Create(sizeof(SSBOVoxelModels), SSBOVoxelModels::Set, SSBOVoxelModels::Binding);
		
		TextureProperties props;
		props.Storage = true;
		props.SamplerWrap = TextureWrap::Clamp;
		m_OutputTexture = Texture2D::Create(ImageFormat::RGBA32F, 1280, 720, nullptr, props);
		m_DepthTexture = Texture2D::Create(ImageFormat::RED32F, 1280, 720, nullptr, props);
		createRaymarchPipeline();

		m_UBVoxelScene.LightDirection = { -0.2f, -1.4f, -1.5f, 1.0f };
		m_UBVoxelScene.LightColor = glm::vec4(1.0f);


		for (uint32_t i = 0; i < 256; i++)
		{
			m_SSBOVoxels.Colors[i] = RandomColor();
		}
	}
	void VoxelRenderer::BeginScene(const glm::mat4& viewProjectionMatrix, const glm::mat4& viewMatrix, const glm::mat4& projection, const glm::vec3& cameraPosition)
	{
		m_CurrentVoxelsCount = 0;

		m_UBVoxelScene.InverseProjection = glm::inverse(projection);
		m_UBVoxelScene.InverseView = glm::inverse(viewMatrix);
		m_UBVoxelScene.CameraPosition = glm::vec4(cameraPosition, 1.0f);
		
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
	void VoxelRenderer::SetColors(const std::array<uint32_t, 256>& colors)
	{
		memcpy(m_SSBOVoxels.Colors, colors.data(), sizeof(SSBOVoxels::Colors));
	}
	void VoxelRenderer::SetColors(const std::array<VoxelColor, 256>& colors)
	{
		memcpy(m_SSBOVoxels.Colors, colors.data(), sizeof(SSBOVoxels::Colors));
	}
	void VoxelRenderer::SubmitMesh(const Ref<VoxelMeshSource>& meshSource, const glm::mat4& transform, float voxelSize)
	{
		auto& drawCommand = m_DrawCommands[meshSource->GetHandle()];
		auto& model = drawCommand.Models.emplace_back();

		const VoxelSubmesh& submesh = meshSource->GetSubmeshes()[0];

		glm::vec3 centerTranslation = -glm::vec3(
			submesh.Width / 2 * voxelSize,
			submesh.Height / 2 * voxelSize,
			submesh.Depth / 2 * voxelSize
		);
		const uint32_t voxelCount = static_cast<uint32_t>(submesh.ColorIndices.size());

		model.Transform = glm::inverse(transform * glm::translate(glm::mat4(1.0f), centerTranslation));
		model.VoxelOffset = m_CurrentVoxelsCount;
		model.Width = submesh.Width;
		model.Height = submesh.Height;
		model.Depth = submesh.Depth;
		model.VoxelSize = voxelSize;


		memcpy(&m_SSBOVoxels.Voxels[m_CurrentVoxelsCount], submesh.ColorIndices.data(), voxelCount * sizeof(uint8_t));
		m_CurrentVoxelsCount += voxelCount;
	}
	
	void VoxelRenderer::OnImGuiRender()
	{
		if (ImGui::Begin("Voxel Renderer"))
		{
			ImGui::DragFloat3("Light Direction", glm::value_ptr(m_UBVoxelScene.LightDirection), 0.1f);
			ImGui::DragFloat3("Light Color", glm::value_ptr(m_UBVoxelScene.LightColor), 0.1f);

			const uint32_t frameIndex = Renderer::GetCurrentFrame();
			float gpuTime = m_CommandBuffer->GetExecutionGPUTime(frameIndex, m_GPUTimeQueries.GPUTime);
			ImGui::Text("GPU Time: %.3fms", gpuTime);
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

	void VoxelRenderer::clear()
	{
		auto imageBarrier = [](Ref<VulkanPipelineCompute> pipeline, Ref<VulkanImage2D> image) {

			Renderer::Submit([pipeline, image]() {
				VkImageMemoryBarrier imageMemoryBarrier = {};
				imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
				imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
				imageMemoryBarrier.image = image->GetImageInfo().Image;
				imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, image->GetSpecification().Mips, 0, 1 };
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				vkCmdPipelineBarrier(
					pipeline->GetActiveCommandBuffer(),
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &imageMemoryBarrier);
					});
		};

		
		Renderer::BeginPipelineCompute(
			m_CommandBuffer,
			m_ClearPipeline,
			m_UniformBufferSet,
			m_StorageBufferSet,
			m_ClearMaterial
		);
		Renderer::DispatchCompute(
			m_ClearPipeline,
			nullptr,
			32, 32, 1,
			PushConstBuffer
			{
				glm::vec4(0.3, 0.2, 0.7, 1.0),
				std::numeric_limits<float>::max()
			}
		);
		imageBarrier(m_ClearPipeline, m_OutputTexture->GetImage());
		imageBarrier(m_ClearPipeline, m_DepthTexture->GetImage());
		Renderer::EndPipelineCompute(m_ClearPipeline);
	}

	void VoxelRenderer::render()
	{
		m_CommandBuffer->Begin();		
		m_GPUTimeQueries.GPUTime = m_CommandBuffer->BeginTimestampQuery();
		clear();
		Renderer::BeginPipelineCompute(
			m_CommandBuffer,
			m_RaymarchPipeline,
			m_UniformBufferSet,
			m_StorageBufferSet,
			m_RaymarchMaterial
		);

		for (const auto& [key, drawCommand] : m_DrawCommands)
		{
			Renderer::DispatchCompute(
				m_RaymarchPipeline,
				nullptr,
				32, 32, 1
			);
		}
		Renderer::EndPipelineCompute(m_RaymarchPipeline);
		m_CommandBuffer->EndTimestampQuery(m_GPUTimeQueries.GPUTime);

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
			m_DepthTexture == Texture2D::Create(ImageFormat::RED32F, m_ViewportSize.x, m_ViewportSize.y, nullptr, props);
			m_RaymarchMaterial->SetImage("o_Image", m_OutputTexture->GetImage());
			m_RaymarchMaterial->SetImage("o_DepthImage", m_DepthTexture->GetImage());

			m_ClearMaterial->SetImage("o_Image", m_OutputTexture->GetImage());
			m_ClearMaterial->SetImage("o_DepthImage", m_DepthTexture->GetImage());
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
		uint32_t modelCount = 0;
		for (auto& [key, drawCommand] : m_DrawCommands)
		{
			for (auto& model : drawCommand.Models)
				m_SSBOVoxelModels.Models[modelCount++] = model;
		}
		m_SSBOVoxelModels.NumModels = modelCount;

		const uint32_t voxelsUpdateSize = sizeof(SSBOVoxels::Colors) + m_CurrentVoxelsCount * sizeof(uint8_t);
		const uint32_t voxelModelsUpdateSize = sizeof(SSBOVoxelModels::NumModels) + sizeof(SSBOVoxelModels::Padding) + modelCount * sizeof(VoxelModel);
		
		m_StorageBufferSet->Update((void*)&m_SSBOVoxelModels, voxelModelsUpdateSize, 0, SSBOVoxelModels::Binding, SSBOVoxelModels::Set);
		m_StorageBufferSet->Update((void*)&m_SSBOVoxels, voxelsUpdateSize, 0, SSBOVoxels::Binding, SSBOVoxels::Set);

	}
	void VoxelRenderer::createRaymarchPipeline()
	{
		Ref<Shader> shader = Shader::Create("Resources/Shaders/RaymarchShader.glsl");
		m_RaymarchMaterial = Material::Create(shader);

		m_RaymarchMaterial->SetImage("o_Image", m_OutputTexture->GetImage());
		m_RaymarchMaterial->SetImage("o_DepthImage", m_DepthTexture->GetImage());

		PipelineComputeSpecification spec;
		spec.Shader = shader;
	
		m_RaymarchPipeline = PipelineCompute::Create(spec);


		Ref<Shader> clearShader = Shader::Create("Resources/Shaders/ImageClearShader.glsl");
		m_ClearMaterial = Material::Create(clearShader);
		m_ClearMaterial->SetImage("o_Image", m_OutputTexture->GetImage());
		m_ClearMaterial->SetImage("o_DepthImage", m_DepthTexture->GetImage());

		spec.Shader = clearShader;
		m_ClearPipeline = PipelineCompute::Create(spec);
	}

}
