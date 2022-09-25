#include "stdafx.h"
#include "LightCullingPass.h"

#include "XYZ/API/Vulkan/Vulkan.h"
#include "XYZ/API/Vulkan/VulkanRenderCommandBuffer.h"

namespace XYZ {

	void LightCullingPass::Init(const LightCullingPassConfiguration& config)
	{
		m_LightsBufferSet = config.LightsBufferSet;
		m_CameraBufferSet = config.CameraBufferSet;

		auto& defaultResources = Renderer::GetDefaultResources();
		m_Material = defaultResources.LightCullingMaterial->GetMaterial();
		m_MaterialInstance = defaultResources.LightCullingMaterial->GetMaterialInstance();
		m_Pipeline = PipelineCompute::Create(m_Material->GetShader());

	}

	void LightCullingPass::Submit(
		const Ref<RenderCommandBuffer>& commandBuffer,
		const Ref<Image2D>& preDepthImage,
		const glm::ivec3& workGroups,
		glm::ivec2 screenSize
	)
	{
		m_Material->SetImage("u_PreDepthMap", preDepthImage);
		m_MaterialInstance->Set("u_ScreenData.u_ScreenSize", screenSize);


		Renderer::BeginPipelineCompute(commandBuffer, m_Pipeline, m_CameraBufferSet, m_LightsBufferSet, m_Material);
		Renderer::DispatchCompute(m_Pipeline, m_MaterialInstance, workGroups.x, workGroups.y, workGroups.z);

		Renderer::Submit([renderCommandBuffer = commandBuffer]() mutable
		{
				const uint32_t frameIndex = Renderer::GetCurrentFrame();
				VkMemoryBarrier barrier = {};
				barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
				barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				VkCommandBuffer vulkanCommandBuffer = (const VkCommandBuffer)renderCommandBuffer.As<VulkanPrimaryRenderCommandBuffer>()->CommandBufferHandle(frameIndex);

				vkCmdPipelineBarrier(vulkanCommandBuffer,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					0,
					1, &barrier,
					0, nullptr,
					0, nullptr);
		});

		Renderer::EndPipelineCompute(m_Pipeline);

		//m_GPUTimeQueries.LightCullingPassQuery = m_CommandBuffer->BeginTimestampQuery();
		//m_CommandBuffer->EndTimestampQuery(m_GPUTimeQueries.LightCullingPassQuery);
	}
}