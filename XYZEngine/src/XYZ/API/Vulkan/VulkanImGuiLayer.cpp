#include "stdafx.h"
#include "VulkanImGuiLayer.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Renderer/Renderer.h"
#include "VulkanContext.h"

#include <imgui.h>
#include <ImGuizmo.h>
#ifndef IMGUI_IMPL_API
#define IMGUI_IMPL_API
#endif

#include <backends/imgui_impl_vulkan_with_textures.h>
#include <backends/imgui_impl_glfw.h>


namespace XYZ
{
    VulkanImGuiLayer::VulkanImGuiLayer()
	    :
		m_DescriptorPool(VK_NULL_HANDLE)
    {
    }

    VulkanImGuiLayer::~VulkanImGuiLayer()
    {
    }

    void VulkanImGuiLayer::OnAttach()
    {
    	IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

		io.FontDefault = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Roboto/Roboto-Regular.ttf", 15.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());

		// Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
		m_SRGBColorSpace = VulkanContext::GetSwapChain().GetFormat().format == VkFormat::VK_FORMAT_B8G8R8A8_SRGB;	
		SetDarkThemeColors();

		Renderer::Submit([this]()
		{
			Application& app = Application::Get();
			GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetWindow());

			auto vulkanContext = VulkanContext::Get();
			const auto device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

			// Create Descriptor Pool
			const VkDescriptorPoolSize pool_sizes[] =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
			};
			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			pool_info.maxSets = 100 * IM_ARRAYSIZE(pool_sizes);
			pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
			pool_info.pPoolSizes = pool_sizes;
			VK_CHECK_RESULT(vkCreateDescriptorPool(device, &pool_info, nullptr, &m_DescriptorPool));
			
			// Setup Platform/Renderer bindings
			ImGui_ImplGlfw_InitForVulkan(window, true);
			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = VulkanContext::GetInstance();
			init_info.PhysicalDevice = VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetVulkanPhysicalDevice();
			init_info.Device = device;
			init_info.QueueFamily = VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetQueueFamilyIndices().Graphics;
			init_info.Queue = VulkanContext::GetCurrentDevice()->GetGraphicsQueue();
			init_info.PipelineCache = nullptr;
			init_info.DescriptorPool = m_DescriptorPool;
			init_info.Allocator = nullptr;
			init_info.MinImageCount = 2;
			init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			const VulkanSwapChain& swapChain = vulkanContext->GetSwapChain();
			init_info.ImageCount = swapChain.GetImageCount();
			init_info.CheckVkResultFn = Utils::VulkanCheckResult;
			ImGui_ImplVulkan_Init(&init_info, swapChain.GetVulkanRenderPass());
			
			// Upload Fonts
			uploadFonts();

			const uint32_t framesInFlight = Renderer::GetConfiguration().FramesInFlight;
			m_ImGuiCommandBuffers.resize(framesInFlight);
			for (uint32_t i = 0; i < framesInFlight; i++)
				m_ImGuiCommandBuffers[i] = VulkanContext::GetCurrentDevice()->CreateSecondaryCommandBuffer();
		});

    }

    void VulkanImGuiLayer::OnDetach()
    {
    	VkDescriptorPool descriptorPool = m_DescriptorPool;
    	Renderer::Submit([descriptorPool]()
		{
			const auto device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

			VK_CHECK_RESULT(vkDeviceWaitIdle(device));
			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
    		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		});
    }

	 void VulkanImGuiLayer::Begin()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
    	if (m_EnableDockspace)
    		beginDockspace();
    }

	 static void CopyImDrawData(ImDrawData& copy, const ImDrawData* drawData)
	 {
		 copy = *drawData;
		 copy.CmdLists = new ImDrawList * [drawData->CmdListsCount];
		 for (int i = 0; i < drawData->CmdListsCount; ++i)
			 copy.CmdLists[i] = drawData->CmdLists[i]->CloneOutput();
	 }

    void VulkanImGuiLayer::End()
    {
    	if (m_EnableDockspace)
    		endDockspace();

    	ImGui::Render();
		ImDrawData copy;
		CopyImDrawData(copy, ImGui::GetDrawData());
		
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
		Renderer::Submit([this, copy]() mutable
		{			
			const VulkanSwapChain& swapChain = VulkanContext::GetSwapChain();

			VkClearValue clearValues[2];
			clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
			clearValues[1].depthStencil = { 1.0f, 0 };

			const uint32_t width = swapChain.GetWidth();
			const uint32_t height = swapChain.GetHeight();

			const uint32_t commandBufferIndex = swapChain.GetCurrentBufferIndex();

			VkCommandBufferBeginInfo drawCmdBufInfo = {};
			drawCmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			drawCmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			drawCmdBufInfo.pNext = nullptr;

			const VkCommandBuffer drawCommandBuffer = swapChain.GetCurrentCommandBuffer();
			VK_CHECK_RESULT(vkBeginCommandBuffer(drawCommandBuffer, &drawCmdBufInfo));

			VkRenderPassBeginInfo renderPassBeginInfo = {};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.pNext = nullptr;
			renderPassBeginInfo.renderPass = swapChain.GetVulkanRenderPass();
			renderPassBeginInfo.renderArea.offset.x = 0;
			renderPassBeginInfo.renderArea.offset.y = 0;
			renderPassBeginInfo.renderArea.extent.width = width;
			renderPassBeginInfo.renderArea.extent.height = height;
			renderPassBeginInfo.clearValueCount = 2; // Color + depth
			renderPassBeginInfo.pClearValues = clearValues;
			renderPassBeginInfo.framebuffer = swapChain.GetCurrentFramebuffer();

			vkCmdBeginRenderPass(drawCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

			VkCommandBufferInheritanceInfo inheritanceInfo = {};
			inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			inheritanceInfo.renderPass = swapChain.GetVulkanRenderPass();
			inheritanceInfo.framebuffer = swapChain.GetCurrentFramebuffer();

			VkCommandBufferBeginInfo cmdBufInfo = {};
			cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
			cmdBufInfo.pInheritanceInfo = &inheritanceInfo;

			VK_CHECK_RESULT(vkBeginCommandBuffer(m_ImGuiCommandBuffers[commandBufferIndex], &cmdBufInfo));

			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = (float)height;
			viewport.height = -(float)height;
			viewport.width = (float)width;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(m_ImGuiCommandBuffers[commandBufferIndex], 0, 1, &viewport);

			VkRect2D scissor = {};
			scissor.extent.width = width;
			scissor.extent.height = height;
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			vkCmdSetScissor(m_ImGuiCommandBuffers[commandBufferIndex], 0, 1, &scissor);

			ImGui_ImplVulkan_RenderDrawData(&copy, m_ImGuiCommandBuffers[commandBufferIndex]);
			for (int i = 0; i < copy.CmdListsCount; ++i)
				IM_DELETE(copy.CmdLists[i]);
			delete[]copy.CmdLists;
			copy.Clear();

			VK_CHECK_RESULT(vkEndCommandBuffer(m_ImGuiCommandBuffers[commandBufferIndex]));

			vkCmdExecuteCommands(drawCommandBuffer, 1, &m_ImGuiCommandBuffers[commandBufferIndex]);

			vkCmdEndRenderPass(drawCommandBuffer);

			VK_CHECK_RESULT(vkEndCommandBuffer(drawCommandBuffer));
		});
    }

    void VulkanImGuiLayer::OnImGuiRender()
    {
    }
	void VulkanImGuiLayer::uploadFonts()
	{
		// Upload Fonts
		auto vulkanContext = VulkanContext::Get();
		const auto device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

		const VkCommandBuffer commandBuffer = vulkanContext->GetCurrentDevice()->GetCommandBuffer(true);
		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		vulkanContext->GetCurrentDevice()->FlushCommandBuffer(commandBuffer);

		VK_CHECK_RESULT(vkDeviceWaitIdle(device));
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
}
