#include "stdafx.h"
#include "VulkanFramebuffer.h"

#include "XYZ/Core/Application.h"
#include "VulkanSwapChain.h"
#include "VulkanContext.h"
#include "VulkanImage.h"
#include "VulkanAllocator.h"
#include "VulkanRendererAPI.h"

namespace XYZ {
	namespace Utils
	{
		VkClearColorValue GlmColorToVkColor(const glm::vec4& color)
		{
			return { color.r, color.g , color.b, color.a };
		}
	}

	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& specs)
		:
		m_Specification(specs),
		m_RenderPass(VK_NULL_HANDLE),
		m_Framebuffer(VK_NULL_HANDLE)
	{
		if (specs.Width == 0 || specs.Height == 0)
		{
			m_Specification.Width = Application::Get().GetWindow().GetWidth();
			m_Specification.Height = Application::Get().GetWindow().GetHeight();
		}

		uint32_t attachmentIndex = 0;
		uint32_t depthAttachmentIndex = 0;
		if (!m_Specification.SwapChainTarget)
		{
			for (auto& attachmentSpec : m_Specification.Attachments)
			{
				if (Utils::IsDepthFormat(attachmentSpec.Format))
				{
					XYZ_ASSERT(m_DepthAttachmentImage.Raw() == nullptr, "");
					ImageSpecification spec;
					spec.Format = attachmentSpec.Format;
					spec.Usage = ImageUsage::Attachment;
					spec.Width = m_Specification.Width;
					spec.Height = m_Specification.Height;
					m_DepthAttachmentImage = Image2D::Create(spec);
					if (attachmentIndex != m_Specification.Attachments.size() - 1) // Swap with last
						std::swap(m_Specification.Attachments.back(), m_Specification.Attachments[attachmentIndex]);
				}
				else
				{
					ImageSpecification spec;
					spec.Format = attachmentSpec.Format;
					spec.Usage = ImageUsage::Attachment;
					spec.Width = m_Specification.Width ;
					spec.Height = m_Specification.Height;
					m_AttachmentImages.emplace_back(Image2D::Create(spec));
				}
				attachmentIndex++;
			}
		}
		Resize(m_Specification.Width, m_Specification.Height, true);
	}
	VulkanFramebuffer::~VulkanFramebuffer()
	{
		release();
	}
	void VulkanFramebuffer::Resize(uint32_t width, uint32_t height, bool forceRecreate)
	{
		if (!forceRecreate && (m_Specification.Width == width && m_Specification.Height == height))
			return;

		if (!m_Specification.SwapChainTarget)
		{
			Invalidate();
		}
		else
		{
			Ref<VulkanContext> context = Renderer::GetAPIContext();
			const VulkanSwapChain& swapChain = context->GetSwapChain();
			m_RenderPass = swapChain.GetVulkanRenderPass();

			m_ClearValues.clear();
			m_ClearValues.emplace_back().color = Utils::GlmColorToVkColor(m_Specification.ClearColor);
		}
	}

	Ref<Image2D> VulkanFramebuffer::GetImage(uint32_t attachmentIndex) const
	{	
		return m_AttachmentImages[attachmentIndex];
	}


	void VulkanFramebuffer::Invalidate()
	{
		Ref< VulkanFramebuffer> instance = this;
		Renderer::Submit([instance]() mutable {
			instance->RT_Invalidate();
		});
	}

	void VulkanFramebuffer::RT_Invalidate()
	{
		auto device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
		release();

		VulkanAllocator allocator("Framebuffer");

		std::vector<VkAttachmentDescription> attachmentDescriptions;

		std::vector<VkAttachmentReference> colorAttachmentReferences;
		VkAttachmentReference depthAttachmentReference;

		m_ClearValues.resize(m_Specification.Attachments.size());

		uint32_t attachmentIndex = 0;
		for (auto attachmentSpec : m_Specification.Attachments)
		{
			VkAttachmentDescription& attachmentDescription = attachmentDescriptions.emplace_back();
			attachmentDescription.flags = 0;
			attachmentDescription.format = Utils::VulkanImageFormat(attachmentSpec.Format);
			attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentDescription.loadOp = m_Specification.ClearOnLoad ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
			attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // TODO: if sampling, needs to be store (otherwise DONT_CARE is fine)
			attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			if (Utils::IsDepthFormat(attachmentSpec.Format))
			{
				Ref<VulkanImage2D> depthAttachmentImage = m_DepthAttachmentImage.As<VulkanImage2D>();
				auto& spec = depthAttachmentImage->GetSpecification();
				spec.Width = m_Specification.Width;
				spec.Height = m_Specification.Height;
				depthAttachmentImage->RT_Invalidate(); // Create immediately
				attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				if (attachmentSpec.Format == ImageFormat::DEPTH24STENCIL8) // Separate layouts requires a "separate layouts" flag to be enabled
				{
					attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // TODO: if not sampling
					attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL; // TODO: if sampling
					depthAttachmentReference = { attachmentIndex, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
				}
				else
				{
					attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL; // TODO: if not sampling
					attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL; // TODO: if sampling
					depthAttachmentReference = { attachmentIndex, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL };
				}
				m_ClearValues[attachmentIndex].depthStencil = { 1.0f, 0 };
			}
			else
			{
				ImageSpecification spec;
				spec.Format = attachmentSpec.Format;
				spec.Usage = ImageUsage::Attachment;
				spec.Width = m_Specification.Width;
				spec.Height = m_Specification.Height;
				Ref<VulkanImage2D> colorAttachment = m_AttachmentImages[attachmentIndex];
				colorAttachment->RT_Invalidate();

				attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				const auto& clearColor = m_Specification.ClearColor;
				m_ClearValues[attachmentIndex].color = { {clearColor.r, clearColor.g, clearColor.b, clearColor.a} };
				colorAttachmentReferences.emplace_back(VkAttachmentReference{ attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			}
			attachmentIndex++;
		}
		
		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = uint32_t(colorAttachmentReferences.size());
		subpassDescription.pColorAttachments = colorAttachmentReferences.data();
		if (m_DepthAttachmentImage.Raw())
			subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;

		// Use subpass dependencies for layout transitions
		std::vector<VkSubpassDependency> dependencies;
		createDependencies(dependencies);

		// Create the actual renderpass
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
		renderPassInfo.pAttachments = attachmentDescriptions.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_RenderPass));

		std::vector<VkImageView> attachments(m_AttachmentImages.size());
		for (uint32_t i = 0; i < m_AttachmentImages.size(); i++)
		{
			Ref<VulkanImage2D> image = m_AttachmentImages[i].As<VulkanImage2D>();
			attachments[i] = image->GetImageInfo().ImageView;
			XYZ_ASSERT(attachments[i], "");
		}
		if (m_DepthAttachmentImage.Raw())
		{
			Ref<VulkanImage2D> image = m_DepthAttachmentImage.As<VulkanImage2D>();
			attachments.emplace_back(image->GetImageInfo().ImageView);
		}
		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = m_RenderPass;
		framebufferCreateInfo.attachmentCount = uint32_t(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.width = m_Specification.Width;
		framebufferCreateInfo.height = m_Specification.Height;
		framebufferCreateInfo.layers = 1;

		VK_CHECK_RESULT(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &m_Framebuffer));
	}
	void VulkanFramebuffer::release() const
	{
		if (m_Framebuffer)
		{
			VkFramebuffer framebuffer = m_Framebuffer;
			VkRenderPass  renderPass = m_RenderPass;
			Renderer::SubmitResource([framebuffer, renderPass]() {
				const auto device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
				vkDestroyFramebuffer(device, framebuffer, nullptr);
				vkDestroyRenderPass(device, renderPass, nullptr);
			});
		}
	}
	void VulkanFramebuffer::createDependencies(std::vector<VkSubpassDependency>& dependencies) const
	{
		if (m_AttachmentImages.size())
		{
			{
				VkSubpassDependency& depedency = dependencies.emplace_back();
				depedency.srcSubpass = VK_SUBPASS_EXTERNAL;
				depedency.dstSubpass = 0;
				depedency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				depedency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				depedency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				depedency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				depedency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}
			{
				VkSubpassDependency& depedency = dependencies.emplace_back();
				depedency.srcSubpass = 0;
				depedency.dstSubpass = VK_SUBPASS_EXTERNAL;
				depedency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				depedency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				depedency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				depedency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				depedency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}
		}

		if (m_DepthAttachmentImage.Raw())
		{
			{
				VkSubpassDependency& depedency = dependencies.emplace_back();
				depedency.srcSubpass = VK_SUBPASS_EXTERNAL;
				depedency.dstSubpass = 0;
				depedency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				depedency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				depedency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				depedency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				depedency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}

			{
				VkSubpassDependency& depedency = dependencies.emplace_back();
				depedency.srcSubpass = 0;
				depedency.dstSubpass = VK_SUBPASS_EXTERNAL;
				depedency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				depedency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				depedency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				depedency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				depedency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}
		}
	}
}
