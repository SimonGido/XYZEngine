#pragma once
#include "OpenXR.h"
#include "XYZ/Renderer/APIContext.h"
#include "XYZ/API/Vulkan/VulkanContext.h"

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <vulkan/vulkan.h>

namespace XYZ {

	struct XYZ_API OpenXRVersion
	{
		uint64_t Major, Minor, Patch;
	};

	struct XYZ_API OpenXRInstanceConfiguration
	{
		std::string				 ApplicationName;
		std::string				 EngineName;
		OpenXRVersion			 Version;
		std::vector<std::string> Extensions;
	};

	class XYZ_API OpenXRInstance : public RefCount
	{
	public:
		using XrEventCallbackFn = std::function<void(const XrEventDataBaseHeader*)>;

		OpenXRInstance(const OpenXRInstanceConfiguration& config);
		~OpenXRInstance();

		bool TryGetSystem();

		void ProcessEvents();


		XrInstance GetXrInstance() const { return m_Instance; }
		XrSystemId GetXrSystemID() const { return m_SystemID; }


		XrEventCallbackFn EventCallback;

	private:
		static std::vector<XrExtensionProperties> listSupportedExtensions();
		
		XrGraphicsRequirementsVulkan2KHR getGraphicsRequirements() const;
		void selectUsedExtensions();
		void setupDebugCallback();

		const XrEventDataBaseHeader* tryReadNextEvent();
	private:
		XrInstance				 m_Instance;
		XrSystemId				 m_SystemID;
		XrDebugUtilsMessengerEXT m_DebugMessenger;
		XrEventDataBuffer		 m_EventDataBuffer;
		bool				     m_SystemCreated;

		std::vector<const char*> m_UsedExtensions;
		std::vector<std::string> m_RequestedExtensions;
	};
}