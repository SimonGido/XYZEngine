#pragma once
#include "OpenXR.h"

#include <openxr/openxr.h>

namespace XYZ {

	struct XYZ_API OpenXRVersion
	{
		uint64_t Major, Minor, Patch;
	};

	class XYZ_API OpenXRInstance
	{
	public:
		OpenXRInstance(const OpenXRVersion& version);

		XrInstance GetXrInstance() const;

	private:
		static std::vector<XrExtensionProperties> listExtensions();

	private:
		XrInstance m_Instance;
	};
}