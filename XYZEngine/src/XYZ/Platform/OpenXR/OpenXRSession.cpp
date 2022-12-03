#include "stdafx.h"
#include "OpenXRSession.h"
#include "XYZ/Renderer/Renderer.h"

#include "XYZ/API/Vulkan/VulkanContext.h"

#include <vulkan/vulkan.h>
#include <openxr/openxr_platform.h>

namespace XYZ {
	OpenXRSession::OpenXRSession(const Ref<OpenXRInstance>& instance)
	{	
		m_Session = createSession(instance);
	}
	OpenXRSession::~OpenXRSession()
	{
		XR_CHECK_RESULT(xrDestroySession(m_Session));
	}
	void OpenXRSession::BeginSession()
	{
		XrSessionBeginInfo beginInfo;
		beginInfo.type = XR_TYPE_SESSION_BEGIN_INFO;
		beginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
		XR_CHECK_RESULT(xrBeginSession(m_Session, &beginInfo));
	}
	void OpenXRSession::EndSession()
	{
		XR_CHECK_RESULT(xrEndSession(m_Session));
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
		PFN_xrGetVulkanGraphicsRequirements2KHR getVulkanGraphicsRequirements;
		XR_CHECK_RESULT(xrGetInstanceProcAddr(instance->GetXrInstance(), "xrGetVulkanGraphicsRequirements2KHR", (PFN_xrVoidFunction*)&getVulkanGraphicsRequirements));

		XrGraphicsRequirementsVulkan2KHR graphicsRequirements;
		graphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN2_KHR;
	
		XR_CHECK_RESULT(getVulkanGraphicsRequirements(instance->GetXrInstance(), instance->GetXrSystemID(), &graphicsRequirements));

		

		PFN_xrGetVulkanGraphicsDevice2KHR getVulkanGraphicsDevice;
		XR_CHECK_RESULT(xrGetInstanceProcAddr(instance->GetXrInstance(), "xrGetVulkanGraphicsDevice2KHR", (PFN_xrVoidFunction*)&getVulkanGraphicsDevice));
		
		Ref<VulkanContext> vulkanContext = Renderer::GetAPIContext().As<VulkanContext>();
		Ref<VulkanPhysicalDevice> physicalDevice = vulkanContext->GetDevice()->GetPhysicalDevice();
		
		XrVulkanGraphicsDeviceGetInfoKHR info;
		info.type = XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR;
		info.systemId = instance->GetXrSystemID();
		info.vulkanInstance = vulkanContext->GetInstance();
		
		VkPhysicalDevice vulkanPhysicalDevice;
		XR_CHECK_RESULT(getVulkanGraphicsDevice(instance->GetXrInstance(), &info, &vulkanPhysicalDevice));


		XrGraphicsBindingVulkan2KHR graphicsBindings;
		graphicsBindings.type = XR_TYPE_GRAPHICS_BINDING_VULKAN2_KHR;
		graphicsBindings.device = vulkanContext->GetDevice()->GetVulkanDevice(); // NOTE: maybe use retrieved device from getVulkanGraphicsDevice
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
}