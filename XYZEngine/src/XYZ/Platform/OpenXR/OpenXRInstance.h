#pragma once
#include "OpenXR.h"

#include <openxr/openxr.h>

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
		OpenXRInstance(const OpenXRInstanceConfiguration& config);
		~OpenXRInstance();

		bool TryGetSystem();


		XrInstance GetXrInstance() const { return m_Instance; }
		XrSystemId GetXrSystemID() const { return m_SystemID; }

	private:
		static std::vector<XrExtensionProperties> listSupportedExtensions();
		void selectUsedExtensions();

	private:
		XrInstance				 m_Instance;
		XrSystemId				 m_SystemID;
		bool				     m_SystemCreated;

		std::vector<const char*> m_UsedExtensions;
		std::vector<std::string> m_RequestedExtensions;
	};
}