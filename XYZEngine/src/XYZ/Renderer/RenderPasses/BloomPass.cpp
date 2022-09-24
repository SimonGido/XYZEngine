#include "stdafx.h"
#include "BloomPass.h"

#include "XYZ/API/Vulkan/VulkanRendererAPI.h"
#include "XYZ/API/Vulkan/VulkanPipelineCompute.h"

namespace XYZ {

	void BloomPass::Init(const BloomPassConfiguration& config, const Ref<RenderCommandBuffer>& commandBuffer)
	{
		m_BloomComputePipeline = PipelineCompute::Create(config.Shader);
		m_BloomComputeMaterial = Material::Create(config.Shader);
		m_BloomComputeMaterialInstance = Ref<MaterialInstance>::Create(m_BloomComputeMaterial);
		m_BloomTexture = config.BloomTexture;
	}

	void BloomPass::SetBloomTextures(const std::array<Ref<Texture2D>, 3>& textures)
	{
		m_BloomTexture = textures;
	}

	void BloomPass::Submit(
		const Ref<RenderCommandBuffer>& commandBuffer, 
		const Ref<Image2D>& lightImage,
		const BloomSettings& bloomSettings,
		glm::ivec2 viewportSize
	)
	{
		XYZ_PROFILE_FUNC("BloomPass::Submit");
		constexpr int prefilter = 0;
		constexpr int downsample = 1;
		constexpr int upsamplefirst = 2;
		constexpr int upsample = 3;

		auto vulkanPipeline = m_BloomComputePipeline;

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

		const uint32_t workGroupSize = 4;
		uint32_t workGroupsX = (uint32_t)glm::ceil(viewportSize.x / workGroupSize);
		uint32_t workGroupsY = (uint32_t)glm::ceil(viewportSize.y / workGroupSize);
		// Renderer::ClearImage(m_CommandBuffer, m_BloomTexture[2]->GetImage());

		Ref<Material> computeMaterial = m_BloomComputeMaterial;
		Ref<MaterialInstance> computeMaterialInst = m_BloomComputeMaterialInstance;
		Renderer::Submit([computeMaterialInst, settings = bloomSettings, prefilter]() mutable {
			computeMaterialInst->Set("u_Uniforms.FilterTreshold", settings.FilterTreshold);
			computeMaterialInst->Set("u_Uniforms.FilterKnee", settings.FilterKnee);
			computeMaterialInst->Set("u_Uniforms.Mode", prefilter);
			//computeMaterial->Set("u_Uniforms.LOD", 0.0f);			
			});
		computeMaterial->SetImage("o_Image", m_BloomTexture[0]->GetImage(), 0);
		computeMaterial->SetImage("u_Texture", lightImage);
		computeMaterial->SetImage("u_BloomTexture", lightImage);

		Renderer::BeginPipelineCompute(commandBuffer, m_BloomComputePipeline, nullptr, nullptr, m_BloomComputeMaterial);
		Renderer::DispatchCompute(m_BloomComputePipeline, computeMaterialInst, workGroupsX, workGroupsY, 1);
		imageBarrier(vulkanPipeline, m_BloomTexture[0]->GetImage());

		Renderer::Submit([computeMaterialInst, downsample]() mutable {
			computeMaterialInst->Set("u_Uniforms.Mode", downsample);
			});

		const uint32_t mips = m_BloomTexture[0]->GetMipLevelCount() - 2;
		for (uint32_t mip = 1; mip < mips; ++mip)
		{
			auto [mipWidth, mipHeight] = m_BloomTexture[0]->GetMipSize(mip);
			workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)workGroupSize);
			workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)workGroupSize);

			computeMaterial->SetImage("o_Image", m_BloomTexture[1]->GetImage(), mip);
			computeMaterial->SetImage("u_Texture", m_BloomTexture[0]->GetImage());
			Renderer::Submit([computeMaterialInst, mip]() mutable {
				computeMaterialInst->Set("u_Uniforms.LOD", (float)mip - 1.0f);
				});

			Renderer::UpdateDescriptors(m_BloomComputePipeline, m_BloomComputeMaterial, nullptr, nullptr);
			Renderer::DispatchCompute(m_BloomComputePipeline, computeMaterialInst, workGroupsX, workGroupsY, 1);
			imageBarrier(vulkanPipeline, m_BloomTexture[1]->GetImage());


			computeMaterial->SetImage("o_Image", m_BloomTexture[0]->GetImage(), mip);
			computeMaterial->SetImage("u_Texture", m_BloomTexture[1]->GetImage());
			Renderer::Submit([computeMaterialInst, mip]() mutable {
				computeMaterialInst->Set("u_Uniforms.LOD", (float)mip);
				});
			Renderer::UpdateDescriptors(m_BloomComputePipeline, m_BloomComputeMaterial, nullptr, nullptr);
			Renderer::DispatchCompute(m_BloomComputePipeline, computeMaterialInst, workGroupsX, workGroupsY, 1);
			imageBarrier(vulkanPipeline, m_BloomTexture[0]->GetImage());
		}
		Renderer::Submit([computeMaterialInst, mips, upsamplefirst]() mutable {
			computeMaterialInst->Set("u_Uniforms.Mode", upsamplefirst);
			computeMaterialInst->Set("u_Uniforms.LOD", mips - 2.0f);
			});

		m_BloomComputeMaterial->SetImage("o_Image", m_BloomTexture[2]->GetImage(), mips - 2);
		m_BloomComputeMaterial->SetImage("u_Texture", m_BloomTexture[0]->GetImage());

		auto [mipWidth, mipHeight] = m_BloomTexture[2]->GetMipSize(mips - 2);
		workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)workGroupSize);
		workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)workGroupSize);

		Renderer::UpdateDescriptors(m_BloomComputePipeline, m_BloomComputeMaterial, nullptr, nullptr);
		Renderer::DispatchCompute(m_BloomComputePipeline, computeMaterialInst, workGroupsX, workGroupsY, 1);
		imageBarrier(vulkanPipeline, m_BloomTexture[2]->GetImage());

		// Upsample stage
		Renderer::Submit([computeMaterialInst, upsample]() mutable {
			computeMaterialInst->Set("u_Uniforms.Mode", upsample);
			});

		for (int32_t mip = mips - 3; mip >= 0; mip--)
		{
			auto [mipWidth, mipHeight] = m_BloomTexture[2]->GetMipSize(mip);
			workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)workGroupSize);
			workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)workGroupSize);

			m_BloomComputeMaterial->SetImage("o_Image", m_BloomTexture[2]->GetImage(), mip);
			m_BloomComputeMaterial->SetImage("u_Texture", m_BloomTexture[0]->GetImage());
			m_BloomComputeMaterial->SetImage("u_BloomTexture", m_BloomTexture[2]->GetImage());
			Renderer::Submit([computeMaterialInst, mip]() mutable {

				computeMaterialInst->Set("u_Uniforms.LOD", (float)mip);
				});

			Renderer::UpdateDescriptors(m_BloomComputePipeline, m_BloomComputeMaterial, nullptr, nullptr);
			Renderer::DispatchCompute(m_BloomComputePipeline, computeMaterialInst, workGroupsX, workGroupsY, 1);
			imageBarrier(vulkanPipeline, m_BloomTexture[2]->GetImage());
		}

		Renderer::EndPipelineCompute(m_BloomComputePipeline);

	}
}