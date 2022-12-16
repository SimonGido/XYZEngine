#include "stdafx.h"
#include "OpenXRSession.h"
#include "XYZ/Renderer/Renderer.h"

#include "XYZ/API/Vulkan/VulkanContext.h"

#include <vulkan/vulkan.h>
#include <openxr/openxr_platform.h>

namespace XYZ {
	OpenXRSession::OpenXRSession(const Ref<OpenXRInstance>& instance)
		:
		m_Running(false)
	{	
		m_Session = createSession(instance);
		createSpace();
	}
	OpenXRSession::~OpenXRSession()
	{
		XR_CHECK_RESULT(xrDestroySpace(m_Space));
		XR_CHECK_RESULT(xrDestroySession(m_Session));
	}
	void OpenXRSession::BeginSession()
	{
		XrSessionBeginInfo beginInfo;
		beginInfo.type = XR_TYPE_SESSION_BEGIN_INFO;
		beginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
		XR_CHECK_RESULT(xrBeginSession(m_Session, &beginInfo));
		m_Running = true;
	}
	void OpenXRSession::EndSession()
	{
		XR_CHECK_RESULT(xrEndSession(m_Session));
		m_Running = false;
	}

	XrSession OpenXRSession::createSession(const Ref<OpenXRInstance>& instance)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::Type::Vulkan: return createVulkanSession(instance);
		}
		return XrSession();
	}
	XrSession OpenXRSession::createVulkanSession(const Ref<OpenXRInstance>& instance)
	{
		Ref<VulkanContext> vulkanContext = Renderer::GetAPIContext().As<VulkanContext>();
		Ref<VulkanPhysicalDevice> physicalDevice = vulkanContext->GetDevice()->GetPhysicalDevice();

		XrGraphicsBindingVulkan2KHR graphicsBindings;
		graphicsBindings.type = XR_TYPE_GRAPHICS_BINDING_VULKAN2_KHR;
		graphicsBindings.device = vulkanContext->GetCurrentDevice()->GetVulkanDevice();
		graphicsBindings.physicalDevice = physicalDevice->GetVulkanPhysicalDevice();
		graphicsBindings.instance = vulkanContext->GetInstance();
		graphicsBindings.queueFamilyIndex = physicalDevice->GetQueueFamilyIndices().Presentation;
		graphicsBindings.queueIndex = 0;
		graphicsBindings.next = nullptr;

		XrSessionCreateInfo sessionCreateInfo;
		sessionCreateInfo.type = XR_TYPE_SESSION_CREATE_INFO;
		sessionCreateInfo.systemId = instance->GetXrSystemID();
		sessionCreateInfo.next = &graphicsBindings;
		sessionCreateInfo.createFlags = 0;

		XrSession session;

		XR_CHECK_RESULT(xrCreateSession(instance->GetXrInstance(), &sessionCreateInfo, &session));
		return session;
	}
	void OpenXRSession::createSpace()
	{
		XrReferenceSpaceCreateInfo spaceCreateInfo;
		spaceCreateInfo.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
		spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;

		spaceCreateInfo.poseInReferenceSpace.position = { 0.0f, 0.0f, 0.0f };
		spaceCreateInfo.poseInReferenceSpace.orientation = { 1.0f, 0.0f, 0.0f, 0.0f };


		XR_CHECK_RESULT(xrCreateReferenceSpace(m_Session, &spaceCreateInfo, &m_Space));
	}
}