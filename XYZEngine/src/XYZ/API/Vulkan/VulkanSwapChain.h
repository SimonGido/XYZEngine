#pragma once
#include "Vulkan.h"
#include "VulkanDevice.h"
#include "VulkanRenderCommandBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanRenderPass.h"

#include <vector>

struct GLFWwindow;
namespace XYZ {

	class VulkanSwapChain
	{
	public:
		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR		Capabilities{};
			std::vector<VkSurfaceFormatKHR> Formats;
			std::vector<VkPresentModeKHR>   PresentModes;
		};
	public:
		VulkanSwapChain();
		
		void Destroy();
		void OnResize(uint32_t width, uint32_t height);
		void BeginFrame();
		void Present();

		void Init(VkInstance instance, const Ref<VulkanDevice>& device);
		void InitSurface(GLFWwindow* windowHandle);
		void Create(uint32_t* width, uint32_t* height, bool vSync);

		Ref<RenderCommandBuffer> GetRenderCommandBuffer() const;
		Ref<RenderPass>			 GetRenderPass()		  const { return m_RenderPass; }
		Ref<VulkanDevice>		 GetDevice()		      const { return m_Device; }
		VkRenderPass			 GetVulkanRenderPass()    const { return m_VulkanRenderPass; }
		
		VkFramebuffer			 GetFramebuffer(uint32_t index) const;
		VkFramebuffer			 GetCurrentFramebuffer()		const;
		
		VkCommandBuffer			 GetCommandBuffer(uint32_t frameIndex) const;
		VkCommandBuffer			 GetCurrentCommandBuffer()			   const { return GetCommandBuffer(m_CurrentImageIndex); }
		VkSemaphore				 GetRenderCompleteSemaphore()		   const { return m_Semaphores[m_CurrentBufferIndex].RenderComplete; }
		VkExtent2D				 GetExtent()						   const { return m_Extent; }
		VkSurfaceFormatKHR		 GetFormat()						   const { return m_Format; }
		VkResult				 GetFenceStatus(uint32_t frameIndex)   const;
		size_t					 GetNumCommandsBuffers()			   const { return m_CommandBuffers.size(); }
		uint32_t				 GetCurrentImageIndex()				   const { return m_CurrentImageIndex; }
		uint32_t				 GetCurrentBufferIndex()			   const { return m_CurrentBufferIndex; }
		uint32_t				 GetWidth()							   const { return m_Extent.width; }
		uint32_t				 GetHeight()						   const { return m_Extent.height; }
		uint32_t				 GetImageCount()					   const { return m_ImageCount;}
	private:
		void getImages();
		void createSyncObjects();
		void findSurfaceFormat();
		void findImageCount();
		void findSwapExtent(uint32_t* width, uint32_t* height);	
		void createSwapChainBuffers();
		void createVulkanRenderPass();
		void createFramebuffers();
		void createCommandPool();
		void createRenderPass();

		void destroySwapChain(VkSwapchainKHR swapChain);
		VkResult queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE) const;

		VkPresentModeKHR			findPresentMode(bool vSync);
		VkCompositeAlphaFlagBitsKHR selectCompositeAlpha() const;
	private:
		GLFWwindow*					   m_WindowHandle;
		VkInstance					   m_Instance;
		Ref<VulkanDevice>			   m_Device;

		// Default swap chain render pass
		Ref<VulkanPrimaryRenderCommandBuffer> m_RenderCommandBuffer;
		Ref<VulkanFramebuffer>				  m_Framebuffer;
		Ref<VulkanRenderPass>				  m_RenderPass;
											

		VkRenderPass				   m_VulkanRenderPass;
		VkSurfaceKHR				   m_Surface;
		VkSwapchainKHR				   m_SwapChain;
		VkSurfaceFormatKHR			   m_Format;
		VkExtent2D					   m_Extent;
		uint32_t					   m_ImageCount;
		uint32_t					   m_CurrentImageIndex;
		uint32_t					   m_CurrentBufferIndex;
		std::vector<VkImage>		   m_Images;
		std::vector<VkFramebuffer>	   m_Framebuffers;
		std::vector<VkCommandBuffer>   m_CommandBuffers;  
		VkCommandPool				   m_CommandPool;
		SwapChainSupportDetails		   m_SwapChainDetails;

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
		bool						 m_RenderPassCreated;
	};
}