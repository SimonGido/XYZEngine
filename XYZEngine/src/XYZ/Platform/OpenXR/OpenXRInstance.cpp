#include "stdafx.h"

#include "OpenXRInstance.h"



namespace XYZ {
	OpenXRInstance::OpenXRInstance(const OpenXRVersion& version)
	{
		XrInstanceCreateInfo info;
		info.type = XR_TYPE_INSTANCE_CREATE_INFO;
		
		XR_MAKE_VERSION(version.Major, version.Minor, version.Patch);

		auto allExtensions = listExtensions();

		XR_CHECK_RESULT(xrCreateInstance(&info, &m_Instance));
	}
	XrInstance OpenXRInstance::GetXrInstance() const
	{
		return m_Instance;
	}
	std::vector<XrExtensionProperties> OpenXRInstance::listExtensions()
	{
		uint32_t extensionCount = 0;
		XR_CHECK_RESULT(xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr));

		std::vector<XrExtensionProperties> extensions(extensionCount);
		for (auto& ext : extensions)
			ext.type = XR_TYPE_EXTENSION_PROPERTIES;		
	
		XR_CHECK_RESULT(xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount, extensions.data()));

		return extensions;
	}
}