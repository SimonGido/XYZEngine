#include "stdafx.h"
#include "OpenXRSwapchain.h"

namespace XYZ {
	OpenXRSwapchain::OpenXRSwapchain(const Ref<OpenXRInstance>& instance, const Ref<OpenXRSession>& session)
		:
		m_Session(session)
	{
		m_ConfigurationViews = enumerateViewConfigurationViews(instance->GetXrInstance(), instance->GetXrSystemID());
		m_Views.resize(m_ConfigurationViews.size(), { XR_TYPE_VIEW });

		selectFormats(session->GetXrSession());
		selectBlendMode(instance->GetXrInstance(), instance->GetXrSystemID());
		createSwapchains(session->GetXrSession());
	}
	OpenXRSwapchain::~OpenXRSwapchain()
	{
		XR_CHECK_RESULT(xrDestroySwapchain(m_Swapchain));
	}
	void OpenXRSwapchain::BeginFrame()
	{
		Renderer::Submit([this]() {
			XrFrameWaitInfo waitInfo;
			waitInfo.type = XR_TYPE_FRAME_WAIT_INFO;


			m_FrameState.type = XR_TYPE_FRAME_STATE;
			XR_CHECK_RESULT(xrWaitFrame(m_Session->GetXrSession(), &waitInfo, &m_FrameState));
		
			XrFrameBeginInfo beginInfo;
			beginInfo.type = XR_TYPE_FRAME_BEGIN_INFO;
			XR_CHECK_RESULT(xrBeginFrame(m_Session->GetXrSession(), &beginInfo));



			if (m_FrameState.shouldRender)
			{
				XrViewLocateInfo viewLocateInfo{ XR_TYPE_VIEW_LOCATE_INFO };
				viewLocateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
				viewLocateInfo.displayTime = m_FrameState.predictedDisplayTime;
				viewLocateInfo.space = m_Session->GetXrSpace();

				XrViewState viewState{ XR_TYPE_VIEW_STATE };
				uint32_t viewCountOutput;
				XR_CHECK_RESULT(xrLocateViews(m_Session->GetXrSession(), &viewLocateInfo, &viewState, m_Views.size(), &viewCountOutput, m_Views.data()));
			}

			acquireAndWaitForSwapchainImage(m_Swapchain);

		});

	}
	void OpenXRSwapchain::EndFrame()
	{
		Renderer::Submit([this]() {

			std::vector<XrCompositionLayerBaseHeader*> layers;
			std::vector<XrCompositionLayerProjectionView> projViews(m_Views.size());
			XrCompositionLayerProjection layerProj{ XR_TYPE_COMPOSITION_LAYER_PROJECTION };

			if (m_FrameState.shouldRender)
			{
				for (size_t i = 0; i < m_Views.size(); ++i)
				{
					XrRect2Di imageRect;
					imageRect.offset.x = 0;
					imageRect.offset.y = 0;

					imageRect.extent.width = m_ConfigurationViews[i].recommendedImageRectWidth;
					imageRect.extent.height = m_ConfigurationViews[i].recommendedImageRectHeight;

					projViews[i].type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
					projViews[i].pose = m_Views[i].pose;
					projViews[i].subImage.swapchain = m_Swapchain;
					projViews[i].subImage.imageArrayIndex = static_cast<uint32_t>(i);
					projViews[i].subImage.imageRect = imageRect;

					// TODO: depth
				}
				layerProj.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
				layerProj.views = projViews.data();
				layerProj.space = m_Session->GetXrSpace();
				layerProj.viewCount = static_cast<uint32_t>(projViews.size());

				layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&layerProj));
			}
			XrSwapchainImageReleaseInfo swapchainImageReleaseInfo;
			swapchainImageReleaseInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO;

			XR_CHECK_RESULT(xrReleaseSwapchainImage(m_Swapchain, &swapchainImageReleaseInfo));

			XrFrameEndInfo frameEndInfo;
			frameEndInfo.type = XR_TYPE_FRAME_END_INFO;
			frameEndInfo.displayTime = m_FrameState.predictedDisplayTime;
			
			frameEndInfo.environmentBlendMode = m_BlendMode;
			frameEndInfo.layerCount = static_cast<uint32_t>(layers.size());
			frameEndInfo.layers = layers.data();

			XR_CHECK_RESULT(xrEndFrame(m_Session->GetXrSession(), &frameEndInfo));
		});
	}
	std::vector<XrViewConfigurationView> OpenXRSwapchain::enumerateViewConfigurationViews(XrInstance instance, XrSystemId systemID)
	{
		uint32_t viewCount = 0;
		XR_CHECK_RESULT(xrEnumerateViewConfigurationViews(
			instance,
			systemID,
			XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, // TODO: input arg
			0,
			&viewCount,
			nullptr
		));

		std::vector<XrViewConfigurationView> views(static_cast<size_t>(viewCount), { XR_TYPE_VIEW_CONFIGURATION_VIEW });
		XR_CHECK_RESULT(xrEnumerateViewConfigurationViews(
			instance,
			systemID,
			XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, // TODO: input arg
			viewCount,
			&viewCount,
			views.data()
		));
		return views;
	}
	void OpenXRSwapchain::selectFormats(XrSession session)
	{
		uint32_t count = 0;
		XR_CHECK_RESULT(xrEnumerateSwapchainFormats(
			session,
			0,
			&count,
			nullptr
		));

		std::vector<int64_t> formats(static_cast<size_t>(count));
		
		XR_CHECK_RESULT(xrEnumerateSwapchainFormats(
			session,
			count,
			&count,
			formats.data()
		));

		for (auto format : formats)
		{
			if (format == VK_FORMAT_B8G8R8A8_UNORM)
			{
				m_ColorFormat = format;
				return;
			}
		}
		m_ColorFormat = formats[0];
	}

	void OpenXRSwapchain::selectBlendMode(XrInstance instance, XrSystemId systemID)
	{
		m_BlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
	}

	void OpenXRSwapchain::createSwapchains(XrSession session)
	{
		auto& view = m_ConfigurationViews[0];

		XrSwapchainCreateInfo createInfo;
		createInfo.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;

		createInfo.arraySize = static_cast<uint32_t>(m_ConfigurationViews.size());
		createInfo.format = m_ColorFormat;
		createInfo.width = view.recommendedImageRectWidth;
		createInfo.height = view.recommendedImageRectHeight;
		createInfo.mipCount = 1;
		createInfo.faceCount = 1;
		createInfo.sampleCount = view.recommendedSwapchainSampleCount;
		createInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
		XR_CHECK_RESULT(xrCreateSwapchain(session, &createInfo, &m_Swapchain));
	}
	void OpenXRSwapchain::getSwapchainImages()
	{
	}

	uint32_t OpenXRSwapchain::acquireAndWaitForSwapchainImage(XrSwapchain swapchain)
	{
		uint32_t swapchainImageIndex = 0;
		XrSwapchainImageAcquireInfo acquireInfo;
		acquireInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO;
		XR_CHECK_RESULT(xrAcquireSwapchainImage(swapchain, &acquireInfo, &swapchainImageIndex));
		
		XrSwapchainImageWaitInfo waitInfo;
		waitInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
		waitInfo.timeout = 0x7fffffffffffffffL; // Infinite

		XR_CHECK_RESULT(xrWaitSwapchainImage(swapchain, &waitInfo));
		
		return swapchainImageIndex;
	}
}