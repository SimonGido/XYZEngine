#pragma once
#include "XYZ/Renderer/APIContext.h"
#include "VulkanSwapChain.h"


namespace XYZ {
	class VulkanContext : public APIContext
	{
	public:
		VulkanContext(GLFWwindow* window);
		virtual ~VulkanContext() override;

		virtual void Init() override;
		virtual void SwapBuffers() override;

	private:
		// Devices
		//Ref<VulkanPhysicalDevice> m_PhysicalDevice;
		//Ref<VulkanDevice>		  m_Device;


		GLFWwindow*				 m_WindowHandle;

		VkDebugReportCallbackEXT m_DebugReportCallback = VK_NULL_HANDLE;
		VkPipelineCache			 m_PipelineCache = nullptr;
		VulkanSwapChain			 m_SwapChain;

		inline static VkInstance s_VulkanInstance;
	};
}