#pragma once
#include "OpenXR.h"
#include "OpenXRInstance.h"


#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

namespace XYZ {
	class OpenXRSession : public RefCount
	{
	public:
		OpenXRSession(const Ref<OpenXRInstance>& instance);
		~OpenXRSession();

		void BeginSession();
		void EndSession();

		XrSession GetXrSession() const { return m_Session; }

	private:
		static XrSession createSession(const Ref<OpenXRInstance>& instance);

		static XrSession createVulkanSession(const Ref<OpenXRInstance>& instance);

	private:
		XrSession m_Session;
	};
}