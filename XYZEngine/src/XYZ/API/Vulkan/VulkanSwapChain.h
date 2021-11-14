#pragma once
#include "Vulkan.h"
#include "VulkanDevice.h"
#include "VulkanRenderCommandBuffer.h"

#include <vector>

struct GLFWwindow;
namespace XYZ {

	class VulkanSwapChain
	{
	public:
		VulkanSwapChain();
		
		void Destroy();
		void OnResize(uint32_t width, uint32_t height);
		void BeginFrame();
		void Present();

		void Init(VkInstance instance, const Ref<VulkanDevice>& device);
		void InitSurface(GLFWwindow* windowHandle);
		void Create(uint32_t* width, uint32_t* height, bool vSync);

		Ref<RenderCommandBuffer> GetRenderCommandBuffer();
		Ref<VulkanDevice>		 GetDevice() { return m_Device; }
		VkRenderPass			 GetRenderPass() const { return m_RenderPass; }
		
		VkFramebuffer			 GetFramebuffer(uint32_t index) const;
		VkFramebuffer			 GetCurrentFramebuffer() const;
		
		VkCommandBuffer			 GetCommandBuffer(uint32_t frameIndex) const;
		VkCommandBuffer			 GetCurrentCommandBuffer() const { return GetCommandBuffer(m_CurrentImageIndex); }
		VkSemaphore				 GetRenderCompleteSemaphore() { return m_Semaphores[m_CurrentBufferIndex].RenderComplete; }
		VkExtent2D				 GetExtent() const { return m_Extent; }
		size_t					 GetNumCommandsBuffers() const { return m_CommandBuffers.size(); }
		uint32_t				 GetCurrentImageIndex() const { return m_CurrentImageIndex; }
		uint32_t				 GetCurrentBufferIndex() const { return m_CurrentBufferIndex; }
		uint32_t				 GetWidth() const { return m_Extent.width; }
		uint32_t				 GetHeight() const { return m_Extent.height; }
	private:
		void getImages();
		void createSyncObjects();
		void findSurfaceFormat();
		void findImageCount();
		void findSwapExtent(uint32_t* width, uint32_t* height);	
		void createSwapChainBuffers();
		void createRenderPass();
		void createFramebuffers();
		void createCommandPool();

		void destroySwapChain(VkSwapchainKHR swapChain);
		VkResult queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);

		VkPresentModeKHR			findPresentMode(bool vSync);
		VkCompositeAlphaFlagBitsKHR selectCompositeAlpha() const;
	private:
		GLFWwindow*					   m_WindowHandle;
		VkInstance					   m_Instance;
		Ref<VulkanDevice>			   m_Device;
		Ref<VulkanRenderCommandBuffer> m_RenderCommandBuffer;
		VkRenderPass				   m_RenderPass;
									 
		VkSurfaceKHR				 m_Surface;
		VkSwapchainKHR				 m_SwapChain;
		VkSurfaceFormatKHR			 m_Format;
		VkExtent2D					 m_Extent;
		uint32_t					 m_ImageCount;
		uint32_t					 m_CurrentImageIndex;
		uint32_t					 m_CurrentBufferIndex;
		std::vector<VkImage>		 m_Images;
		std::vector<VkFramebuffer>	 m_Framebuffers;
		std::vector<VkCommandBuffer> m_CommandBuffers;  
		VkCommandPool				 m_CommandPool;

		struct Semaphores
		{
			// Swap chain
			VkSemaphore PresentComplete;
			// Command buffer
			VkSemaphore RenderComplete;
		};

		std::vector<VkFence>		 m_WaitFences;
		std::vector<Semaphores>		 m_Semaphores;

		struct SwapChainBuffer
		{
			VkImage		Image;
			VkImageView View;
		};
		std::vector<SwapChainBuffer> m_Buffers;
		bool						 m_VSync;
	};
}