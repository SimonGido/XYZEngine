#pragma once
#include "XYZ/Renderer/Framebuffer.h"

#include <vulkan/vulkan.h>

namespace XYZ {
	class VulkanFramebuffer : public Framebuffer
	{
	public:
		VulkanFramebuffer(const FramebufferSpecification& specs);
		virtual ~VulkanFramebuffer() override;

		virtual void					Resize(uint32_t width, uint32_t height, bool forceRecreate = false) override;
		virtual const uint32_t			GetNumColorAttachments()  const override { return m_Specification.SwapChainTarget ? 1 : m_AttachmentImages.size(); }
		virtual Ref<Image2D>			GetImage(uint32_t attachmentIndex = 0) const override;
		virtual Ref<Image2D>			GetDepthImage() const override { return m_DepthAttachmentImage; }
		virtual void				    SetSpecification(const FramebufferSpecification& specs, bool recreate = false) override;
		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }


		VkRenderPass					 GetRenderPass() const { return m_RenderPass; }
		VkFramebuffer					 GetFramebuffer() const { return m_Framebuffer; }
		const std::vector<VkClearValue>& GetVulkanClearValues() const { return m_ClearValues; }
		bool							 HasDepthAttachment() const { return m_DepthAttachmentImage.Raw(); }
		Ref<Image2D>					 GetDepthAttachment() const { return m_DepthAttachmentImage; }
		
		void Invalidate();
		void RT_Invalidate();
	private:
		void release() const;
		void createDependencies(std::vector<VkSubpassDependency>& dependencies) const;

	private:
		FramebufferSpecification  m_Specification;
		VkRenderPass			  m_RenderPass;
		VkFramebuffer			  m_Framebuffer;
		Ref<Image2D>			  m_DepthAttachmentImage;
		std::vector<Ref<Image2D>> m_AttachmentImages;
		std::vector<VkClearValue> m_ClearValues;

		friend class VulkanSwapChain;
	};
}