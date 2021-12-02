#pragma once
#include "XYZ/Renderer/APIContext.h"
#include "VulkanSwapChain.h"


namespace XYZ {
	class VulkanContext : public APIContext
	{
	public:
		VulkanContext();
		virtual ~VulkanContext() override;

		virtual void Init(GLFWwindow* window) override;
		virtual void Shutdown() override;

		virtual void CreateSwapChain(uint32_t* width, uint32_t* height, bool vSync) override;
		virtual void SwapBuffers() override;
		virtual void BeginFrame() override;
		virtual void OnResize(uint32_t width, uint32_t height) override;
		virtual uint32_t GetCurrentFrame() const override { return m_SwapChain.GetCurrentBufferIndex();}

		virtual Ref<RenderCommandBuffer> GetRenderCommandBuffer() override;

		Ref<VulkanDevice>				 GetDevice() const { return m_Device; }
		static VkInstance				 GetInstance() { return s_VulkanInstance; }
		static Ref<VulkanContext>		 Get();
		static Ref<VulkanDevice>		 GetCurrentDevice() { return Get()->GetDevice(); }
		static VulkanSwapChain&			 GetSwapChain() { return Get()->m_SwapChain; }
	private:
		void setupDebugCallback();
		void setupDevice();
	private:
		// Devices
		Ref<VulkanDevice>		  m_Device;

		GLFWwindow*				  m_WindowHandle;

		VkDebugReportCallbackEXT  m_DebugReportCallback;
		VkPipelineCache			  m_PipelineCache;
		VulkanSwapChain			  m_SwapChain;

		inline static VkInstance s_VulkanInstance;
	};
}