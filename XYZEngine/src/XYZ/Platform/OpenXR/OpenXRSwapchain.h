#pragma once
#include "OpenXRInstance.h"
#include "OpenXRSession.h"

namespace XYZ {

	class XYZ_API OpenXRSwapchain : public RefCount
	{
	public:
		OpenXRSwapchain(const Ref<OpenXRInstance>& instance, const Ref<OpenXRSession>& session);
		~OpenXRSwapchain();

		void BeginFrame();
		void EndFrame();

	private:
		static std::vector<XrViewConfigurationView> enumerateViewConfigurationViews(XrInstance instance, XrSystemId systemID);
		
		void selectFormats(XrSession session);
		void selectBlendMode(XrInstance instance, XrSystemId systemID);
		void createSwapchains(XrSession session);
		void getSwapchainImages();

		uint32_t acquireAndWaitForSwapchainImage(XrSwapchain swapchain);

	private:
		Ref<OpenXRSession> m_Session;
		int64_t			   m_ColorFormat;
		XrSwapchain		   m_Swapchain;
		XrFrameState	   m_FrameState;
		XrEnvironmentBlendMode m_BlendMode;

		std::vector<XrSwapchainImageVulkan2KHR>    m_Images;
		std::vector<XrViewConfigurationView>	   m_ConfigurationViews;
		std::vector<XrView> m_Views;
	};
}