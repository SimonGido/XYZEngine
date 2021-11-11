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

		Ref<VulkanDevice> GetDevice() { return m_Device; }
		
		static VkInstance		  GetInstance() { return s_VulkanInstance; }
		static Ref<VulkanContext> Get();
		static Ref<VulkanDevice>  GetCurrentDevice() { return Get()->GetDevice(); }

	private:
		void setupDebugCallback();
		void setupDevices();
	private:
		// Devices
		Ref<VulkanPhysicalDevice> m_PhysicalDevice;
		Ref<VulkanDevice>		  m_Device;

		GLFWwindow*				  m_WindowHandle;

		VkDebugReportCallbackEXT  m_DebugReportCallback;
		VkPipelineCache			  m_PipelineCache;
		VulkanSwapChain			  m_SwapChain;

		inline static VkInstance s_VulkanInstance;
	};
}