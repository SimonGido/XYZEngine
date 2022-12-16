#include "stdafx.h"

#include "OpenXRInstance.h"

#include "XYZ/Renderer/Renderer.h"

#include "XYZ/API/Vulkan/VulkanContext.h"



namespace XYZ {

	static XrBool32 XRAPI_CALL XrDebugReportCallback(
		XrDebugUtilsMessageSeverityFlagsEXT              messageSeverity,
		XrDebugUtilsMessageTypeFlagsEXT                  messageTypes,
		const XrDebugUtilsMessengerCallbackDataEXT* callbackData,
		void* userData
	)
	{
		if (messageSeverity | XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			XYZ_CORE_INFO("OpenXRDebugCallback:\n  Function: {0}\n  Message: {1}", callbackData->functionName, callbackData->message);
		}
		else if (messageSeverity | XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			XYZ_CORE_WARN("OpenXRDebugCallback:\n  Function: {0}\n  Message: {1}", callbackData->functionName, callbackData->message);
		}
		else if (messageSeverity | XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			XYZ_CORE_ERROR("OpenXRDebugCallback:\n  Function: {0}\n  Message: {1}", callbackData->functionName, callbackData->message);
		}
			
		return XR_FALSE;
	}

	std::vector<const char*> ParseExtensionString(char* names) 
	{
		std::vector<const char*> list;
		while (*names != 0) 
		{
			list.push_back(names);
			while (*(++names) != 0)
			{
				if (*names == ' ') 
				{
					*names++ = '\0';
					break;
				}
			}
		}
		return list;
	}

	static const char* SelectAPIExtension()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::Type::Vulkan: return "XR_KHR_vulkan_enable2";
		}

		return nullptr;
	}

	OpenXRInstance::OpenXRInstance(const OpenXRInstanceConfiguration& config)
		:
		m_RequestedExtensions(config.Extensions),
		m_SystemCreated(false)
	{

		selectUsedExtensions();
		m_UsedExtensions.push_back(SelectAPIExtension());
		#ifdef XYZ_DEBUG
			m_UsedExtensions.push_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
		#endif
		
		auto version = XR_MAKE_VERSION(config.Version.Major, config.Version.Minor, config.Version.Patch);
		
		XrInstanceCreateInfo info;
		
		info.type = XR_TYPE_INSTANCE_CREATE_INFO;
		memcpy(info.applicationInfo.applicationName, config.ApplicationName.c_str(), config.ApplicationName.size());
		info.applicationInfo.applicationName[config.ApplicationName.size()] = '\0';

		memcpy(info.applicationInfo.engineName, config.EngineName.c_str(), config.EngineName.size());
		info.applicationInfo.engineName[config.EngineName.size()] = '\0';


		info.applicationInfo.apiVersion = version;
		
		info.enabledExtensionNames = m_UsedExtensions.data();
		info.enabledExtensionCount = static_cast<uint32_t>(m_UsedExtensions.size());
		info.enabledApiLayerCount = 0;
		info.createFlags = 0;
		info.next = nullptr;

		XR_CHECK_RESULT(xrCreateInstance(&info, &m_Instance));
		TryGetSystem();
		getGraphicsRequirements(); // It must be called at least once before creating session
	
#ifdef XYZ_DEBUG
		setupDebugCallback();
#endif
	}

	OpenXRInstance::~OpenXRInstance()
	{
#ifdef XYZ_DEBUG
		XR_CHECK_RESULT(Utils::ExecuteFunctionXr(m_Instance, "xrDestroyDebugUtilsMessengerEXT",
			// Args
			m_DebugMessenger
		));
#endif

		XR_CHECK_RESULT(xrDestroyInstance(m_Instance));
		PFN_xrDestroyDebugUtilsMessengerEXT;

	}

	bool OpenXRInstance::TryGetSystem()
	{
		XrSystemGetInfo systemGetInfo;
		systemGetInfo.type = XR_TYPE_SYSTEM_GET_INFO;
		systemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
		
		XrResult result = xrGetSystem(m_Instance, &systemGetInfo, &m_SystemID);
		if (result != XR_SUCCESS)
		{
			XYZ_ASSERT(result == XR_ERROR_FORM_FACTOR_UNAVAILABLE, "Failed to create XrSystem");
			XYZ_CORE_WARN("Not headset detected");
		}
		else
		{
			m_SystemCreated = true;
		}
		return true;
	}

	void OpenXRInstance::ProcessEvents()
	{
		XrEventDataBuffer eventDataBuffer;
		eventDataBuffer.type = XR_TYPE_EVENT_DATA_BUFFER;

		while (const XrEventDataBaseHeader* event = tryReadNextEvent())
		{
			if (EventCallback)
				EventCallback(event);
		}
	}

	std::vector<XrExtensionProperties> OpenXRInstance::listSupportedExtensions()
	{
		uint32_t extensionCount = 0;
		XR_CHECK_RESULT(xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr));

		std::vector<XrExtensionProperties> extensions(extensionCount);
		for (auto& ext : extensions)
			ext.type = XR_TYPE_EXTENSION_PROPERTIES;		
	
		XR_CHECK_RESULT(xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount, extensions.data()));

		return extensions;
	}
	XrGraphicsRequirementsVulkan2KHR OpenXRInstance::getGraphicsRequirements() const
	{
		PFN_xrGetVulkanGraphicsRequirements2KHR getVulkanGraphicsRequirements;
		XR_CHECK_RESULT(xrGetInstanceProcAddr(m_Instance, "xrGetVulkanGraphicsRequirements2KHR", (PFN_xrVoidFunction*)&getVulkanGraphicsRequirements));


		XrGraphicsRequirementsVulkan2KHR graphicsRequirements;
		graphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN2_KHR;

		XR_CHECK_RESULT(getVulkanGraphicsRequirements(m_Instance, m_SystemID, &graphicsRequirements));


		return graphicsRequirements;
	}
	void OpenXRInstance::selectUsedExtensions()
	{
		auto supportedExtensions = listSupportedExtensions();

		for (auto& extName : m_RequestedExtensions)
		{
			auto comparator = [&extName](const XrExtensionProperties& ext) -> bool {
				if (ext.extensionName == extName)
					return true;
				return false;
			};

			auto it = std::find_if(supportedExtensions.begin(), supportedExtensions.end(), comparator);
			if (it != supportedExtensions.end())
			{
				m_UsedExtensions.push_back(extName.c_str());
			}
			else
			{
				XYZ_CORE_WARN("Extension {} is not supported", extName);
			}
		}
	}
	void OpenXRInstance::setupDebugCallback()
	{
		
		XrDebugUtilsMessengerCreateInfoEXT messengerCreateInfo;
		messengerCreateInfo.type = XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		messengerCreateInfo.messageTypes =
			XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
			| XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT;

		messengerCreateInfo.messageSeverities =
			XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
			XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

		messengerCreateInfo.userCallback = XrDebugReportCallback;
		messengerCreateInfo.userData = nullptr;

		XR_CHECK_RESULT(Utils::ExecuteFunctionXr(m_Instance, "xrCreateDebugUtilsMessengerEXT",
			//Args
			m_Instance,
			&messengerCreateInfo,
			&m_DebugMessenger
		));
	}
	const XrEventDataBaseHeader* OpenXRInstance::tryReadNextEvent()
	{
		XrEventDataBaseHeader* baseHeader = reinterpret_cast<XrEventDataBaseHeader*>(&m_EventDataBuffer);
		*baseHeader = { XR_TYPE_EVENT_DATA_BUFFER };
		const XrResult xr = xrPollEvent(m_Instance, &m_EventDataBuffer);
		if (xr == XR_SUCCESS) 
		{
			if (baseHeader->type == XR_TYPE_EVENT_DATA_EVENTS_LOST) 
			{
				const XrEventDataEventsLost* const eventsLost = reinterpret_cast<const XrEventDataEventsLost*>(baseHeader);
				XYZ_CORE_INFO("{} events lost", eventsLost->lostEventCount);
			}
			return baseHeader;
		}
		if (xr == XR_EVENT_UNAVAILABLE) 
		{
			return nullptr;
		}
	}
}