#include "stdafx.h"
#include "VulkanContext.h"

namespace XYZ {
	#ifdef XYZ_DEBUG
	static bool s_Validation = true;
	#else
	static bool s_Validation = false; // Let's leave this on for now...
	#endif

	namespace Utils {
		static std::vector<VkExtensionProperties> GetSupportedExtensions()
		{
			uint32_t extensionCount = 0;
			vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
			std::vector<VkExtensionProperties> extensions(extensionCount);
			vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
			return extensions;
		}
	}


	VulkanContext::VulkanContext(GLFWwindow* window)
		:
		m_WindowHandle(window)
	{
	}
	VulkanContext::~VulkanContext()
	{
		vkDestroyInstance(s_VulkanInstance, nullptr);
		s_VulkanInstance = nullptr;
	}
	void VulkanContext::Init()
	{
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pApplicationName = "XYZEngine";
		appInfo.pEngineName = "XYZEngine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;
	
		#define VK_KHR_WIN32_SURFACE_EXTENSION_NAME "VK_KHR_win32_surface"

		std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
		if (s_Validation)
		{
			instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		}

		VkValidationFeatureEnableEXT enables[] = { VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT };
		VkValidationFeaturesEXT features = {};
		features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
		features.enabledValidationFeatureCount = 1;
		features.pEnabledValidationFeatures = enables;

		VkInstanceCreateInfo instanceCreateInfo = {};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pNext = nullptr;// &features;
		instanceCreateInfo.pApplicationInfo = &appInfo;
		instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
		
		VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &s_VulkanInstance));
	}
	void VulkanContext::SwapBuffers()
	{
	}
}