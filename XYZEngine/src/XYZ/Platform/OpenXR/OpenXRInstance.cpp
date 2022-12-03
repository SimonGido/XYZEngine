#include "stdafx.h"

#include "OpenXRInstance.h"

#include "XYZ/Renderer/Renderer.h"

namespace XYZ {

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
	}

	OpenXRInstance::~OpenXRInstance()
	{
		XR_CHECK_RESULT(xrDestroyInstance(m_Instance));
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
}