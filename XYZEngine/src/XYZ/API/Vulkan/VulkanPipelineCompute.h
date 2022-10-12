#pragma once
#include "XYZ/Renderer/PipelineCompute.h"

#include "VulkanShader.h"
#include "VulkanTexture.h"
#include "VulkanRenderCommandBuffer.h"

namespace XYZ {
	class VulkanPipelineCompute : public PipelineCompute
	{
	public:
		VulkanPipelineCompute(Ref<Shader> computeShader);
		virtual ~VulkanPipelineCompute() override;

		virtual void		Invalidate() override;
		virtual void		Begin(Ref<RenderCommandBuffer> renderCommandBuffer = nullptr) override;
		virtual void		End() override;
		virtual Ref<Shader> GetShader() const override { return m_Shader; }

	
		void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

		void SetPushConstants(const void* data, uint32_t size);
		void CreatePipeline();

		VkCommandBuffer  GetActiveCommandBuffer()  const { return m_ActiveComputeCommandBuffer; }
		VkPipelineLayout GetVulkanPipelineLayout() const { return m_ComputePipelineLayout; }
	
	private:
		void destroy();
		void RT_createPipeline();
	private:
		Ref<VulkanShader> m_Shader;

		VkPipelineLayout m_ComputePipelineLayout = nullptr;
		VkPipelineCache m_PipelineCache = nullptr;
		VkPipeline m_ComputePipeline = nullptr;

		VkCommandBuffer m_ActiveComputeCommandBuffer = nullptr;

		bool m_UsingGraphicsQueue = false;
	};
}