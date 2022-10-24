#include "stdafx.h"
#include "VulkanImGuiLayer.h"
#include "VulkanContext.h"

#include "XYZ/Core/Application.h"
#include "XYZ/Renderer/Renderer.h"

#include "XYZ/Utils/StringUtils.h"
#include "XYZ/ImGui/ImGui.h"

#include <ImGuizmo.h>
#ifndef IMGUI_IMPL_API
#define IMGUI_IMPL_API
#endif

#include <backends/imgui_impl_vulkan_with_textures.h>
#include <backends/imgui_impl_glfw.h>

#include <glfw/glfw3.h>

namespace XYZ {

	static std::vector<VkCommandBuffer> s_ImGuiCommandBuffers;


	static ImGuiID GetImageID(const Ref<VulkanImage2D>& image)
	{
		const auto& imageInfo = image->GetImageInfo();
		if (!imageInfo.ImageView)
			return 0;

		return (ImGuiID)((((uint64_t)imageInfo.ImageView) >> 32) ^ (uint32_t)imageInfo.ImageView);
	}

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
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows


	

		io.Fonts->AddFontFromFileTTF("Resources/Fonts/Roboto/Roboto-Black.ttf", 15.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
		io.Fonts->AddFontFromFileTTF("Resources/Fonts/Roboto/Roboto-Light.ttf", 15.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
		io.Fonts->AddFontFromFileTTF("Resources/Fonts/Roboto/Roboto-Medium.ttf", 15.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
		io.Fonts->AddFontFromFileTTF("Resources/Fonts/Roboto/Roboto-Thin.ttf", 15.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
		io.FontDefault = io.Fonts->AddFontFromFileTTF("Resources/Fonts/Roboto/Roboto-Regular.ttf", 15.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());

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

		const VulkanSwapChain& swapChain = VulkanContext::GetSwapChain();

		m_DescriptorAllocator = Ref<VulkanDescriptorAllocator>::Create();
		m_DescriptorAllocator->Init();
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

			const VulkanSwapChain& swapChain = vulkanContext->GetSwapChain();
			init_info.MinImageCount = 2;
			init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			init_info.ImageCount = swapChain.GetImageCount();
			init_info.CheckVkResultFn = Utils::VulkanCheckResult;
			ImGui_ImplVulkan_Init(&init_info, swapChain.GetVulkanRenderPass());
			
			// Upload Fonts
			RT_uploadFonts();
		
			const uint32_t framesInFlight = Renderer::GetConfiguration().FramesInFlight;
			s_ImGuiCommandBuffers.resize(framesInFlight);
			for (uint32_t i = 0; i < framesInFlight; i++)
				s_ImGuiCommandBuffers[i] = VulkanContext::GetCurrentDevice()->CreateSecondaryCommandBuffer();

		});
    }

    void VulkanImGuiLayer::OnDetach()
    {
    	VkDescriptorPool descriptorPool = m_DescriptorPool;
		m_DescriptorAllocator->Shutdown();
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
		addWaitingFonts();
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
    	if (m_EnableDockspace)
    		beginDockspace();

		m_DescriptorAllocator->TryResetFull();
    }
	
	static void CopyImDrawData(ImDrawData& copy, const ImDrawData* drawData)
	{
		copy = *drawData;
		copy.CmdLists = new ImDrawList * [drawData->CmdListsCount];
		for (int i = 0; i < drawData->CmdListsCount; ++i)
		 copy.CmdLists[i] = drawData->CmdLists[i]->CloneOutput();
	}

	static void DeleteImDrawData(ImDrawData& data)
	{
		for (int i = 0; i < data.CmdListsCount; ++i)
			IM_DELETE(data.CmdLists[i]);
		delete[]data.CmdLists;
		data.Clear();
	}

    void VulkanImGuiLayer::End()
    {
    	if (m_EnableDockspace)
    		endDockspace();
	
		ImGui::Render();
		ImDrawData copy;
		CopyImDrawData(copy, ImGui::GetDrawData());
		
		ImGuiIO& io = ImGui::GetIO();
		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
		
		Renderer::Submit([this, copy, descriptors = std::move(m_ImGuiImageDescriptors)]() mutable {
			
			RT_beginRender();
			for (auto& [id, desc] : descriptors)
			{
				const auto& imageInfo = desc.Image->GetImageInfo();
				ImGui_ImplVulkan_UpdateTextureInfo(desc.Descriptor, imageInfo.Sampler, imageInfo.ImageView, desc.Image->GetDescriptor().imageLayout);
			}
			const uint32_t currentFrame = Renderer::GetCurrentFrame();
			ImGui_ImplVulkan_RenderDrawData(&copy, s_ImGuiCommandBuffers[currentFrame]);

			RT_endRender();
			
			DeleteImDrawData(copy);
		});
    }

	void VulkanImGuiLayer::AddFont(const ImGuiFontConfig& config)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImFontAtlas* atlas = io.Fonts;
		for (int i = 0; i < atlas->Fonts.Size; i++)
		{
			ImFont* font = atlas->Fonts[i];
			std::string fontName = font->ConfigData ? font->ConfigData[0].Name : "";
			std::string newFontName = Utils::GetFilename(config.Filepath);
			if (newFontName == fontName )
			{
				if (font->ConfigData && font->ConfigData->SizePixels == config.SizePixels)
				{
					XYZ_CORE_WARN("Font with name {} and pixel size {} already exists", fontName.c_str(), config.SizePixels);
					return;
				}
			}
		}
		m_AddFonts.push(config);
	}

	bool VulkanImGuiLayer::IsMultiViewport() const
	{
		return ImGui::GetPlatformIO().Viewports.size() > 1;
	}

    void VulkanImGuiLayer::OnImGuiRender()
    {
    }

	ImTextureID VulkanImGuiLayer::AddImage(const Ref<VulkanImage2D>& image)
	{	
		const uint32_t frame = Renderer::GetCurrentFrame();
		const ImGuiID id = GetImageID(image);
		auto it = m_ImGuiImageDescriptors.find(id);
		if (it != m_ImGuiImageDescriptors.end())
			return it->second.Descriptor;

		
		VkDescriptorSet newImageDescriptor = m_DescriptorAllocator->RT_Allocate(ImGui_ImplVulkan_GetDescriptorSetLayout());
		m_ImGuiImageDescriptors[id] = { image, newImageDescriptor };	
		return newImageDescriptor;
	}
	void VulkanImGuiLayer::addWaitingFonts()
	{
		if (!m_AddFonts.empty())
		{
			Renderer::SubmitAndWait([this]() {
				while (!m_AddFonts.empty())
				{
					const auto fontConfig = m_AddFonts.back();
					m_AddFonts.pop();
					ImGui::GetIO().Fonts->AddFontFromFileTTF(fontConfig.Filepath.c_str(), fontConfig.SizePixels, nullptr, fontConfig.GlyphRange);
					m_FontsLoaded.push_back(fontConfig);
				}
				RT_uploadFonts();
			});
		}
	}
	void VulkanImGuiLayer::RT_uploadFonts()
	{
		// Upload Fonts
		auto vulkanContext = VulkanContext::Get();
		const auto device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

		ImGui_ImplVulkan_DestroyFontImage();
		const VkCommandBuffer commandBuffer = vulkanContext->GetCurrentDevice()->GetCommandBuffer(true);
		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		vulkanContext->GetCurrentDevice()->FlushCommandBuffer(commandBuffer);
		
		VK_CHECK_RESULT(vkDeviceWaitIdle(device));
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
	void VulkanImGuiLayer::RT_beginRender()
	{
		VulkanSwapChain& swapChain = VulkanContext::Get()->GetSwapChain();
		
		VkClearValue clearValues[2];
		clearValues[0].color = { {0.1f, 0.1f,0.1f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		const uint32_t width = swapChain.GetWidth();
		const uint32_t height = swapChain.GetHeight();
	
		const uint32_t commandBufferIndex = swapChain.GetCurrentBufferIndex();

		VkCommandBufferBeginInfo drawCmdBufInfo = {};
		drawCmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		drawCmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		drawCmdBufInfo.pNext = nullptr;

		VkCommandBuffer drawCommandBuffer = swapChain.GetCurrentCommandBuffer();
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

		VK_CHECK_RESULT(vkBeginCommandBuffer(s_ImGuiCommandBuffers[commandBufferIndex], &cmdBufInfo));

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = (float)height;
		viewport.height = -(float)height;
		viewport.width = (float)width;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(s_ImGuiCommandBuffers[commandBufferIndex], 0, 1, &viewport);

		VkRect2D scissor = {};
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(s_ImGuiCommandBuffers[commandBufferIndex], 0, 1, &scissor);
	}
	void VulkanImGuiLayer::RT_endRender()
	{
		VulkanSwapChain& swapChain = VulkanContext::Get()->GetSwapChain();
		VkCommandBuffer drawCommandBuffer = swapChain.GetCurrentCommandBuffer();
		const uint32_t commandBufferIndex = swapChain.GetCurrentBufferIndex();

		VK_CHECK_RESULT(vkEndCommandBuffer(s_ImGuiCommandBuffers[commandBufferIndex]));

		vkCmdExecuteCommands(drawCommandBuffer, 1u, &s_ImGuiCommandBuffers[commandBufferIndex]);

		vkCmdEndRenderPass(drawCommandBuffer);

		VK_CHECK_RESULT(vkEndCommandBuffer(drawCommandBuffer));
	}
}
