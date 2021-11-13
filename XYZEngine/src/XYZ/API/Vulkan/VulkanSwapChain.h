#pragma once
#include "Vulkan.h"
#include "VulkanDevice.h"

#include <vector>

struct GLFWwindow;
namespace XYZ {

	class VulkanSwapChain
	{
	public:
		VulkanSwapChain();
		
		void Destroy();
		void OnResize(uint32_t width, uint32_t height);

		void Init(VkInstance instance, const Ref<VulkanDevice>& device);
		void InitSurface(GLFWwindow* windowHandle);
		void Create(uint32_t* width, uint32_t* height, bool vSync);

	private:
		void getImages();
		void findSurfaceFormat();
		void findImageCount();
		void findSwapExtent(uint32_t* width, uint32_t* height);	
		void createSwapChainBuffers();
		void createRenderPass();
		void createFramebuffers();
		void createCommandPool();

		void destroySwapChain(VkSwapchainKHR swapChain);

		VkPresentModeKHR			findPresentMode(bool vSync);
		VkCompositeAlphaFlagBitsKHR selectCompositeAlpha() const;
	private:
		GLFWwindow*					 m_WindowHandle;
		VkInstance					 m_Instance;
		Ref<VulkanDevice>			 m_Device;
		VkRenderPass				 m_RenderPass;
									 
		VkSurfaceKHR				 m_Surface;
		VkSwapchainKHR				 m_SwapChain;
		VkSurfaceFormatKHR			 m_Format;
		VkExtent2D					 m_Extent;
		uint32_t					 m_ImageCount;

		std::vector<VkImage>		 m_Images;
		std::vector<VkFramebuffer>	 m_Framebuffers;
		std::vector<VkCommandBuffer> m_CommandBuffers;
		VkCommandPool				 m_CommandPool;



		struct SwapChainBuffer
		{
			VkImage		Image;
			VkImageView View;
		};
		std::vector<SwapChainBuffer> m_Buffers;
		bool						 m_VSync;
	};
}