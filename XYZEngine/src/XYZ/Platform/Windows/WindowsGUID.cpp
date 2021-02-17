#include "stdafx.h"
#include "XYZ/Core/GUID.h"

#include <array>

#ifdef XYZ_PLATFORM_WINDOWS
#include <Rpc.h>


namespace XYZ {
	static std::string GuidToString(UUID guid) 
	{
		std::array<char, 40> output;
		snprintf(output.data(), output.size(), "{%08X-%04hX-%04hX-%02X%02X-%02X%02X%02X%02X%02X%02X}", guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
		return std::string(output.data());
	}
	GUID::GUID()
	{
		UUID uuid;
		auto result = CoCreateGuid(&uuid);
		m_UUID = GuidToString(uuid);
	}
	GUID::GUID(const std::string& id)
		: m_UUID(id)
	{
		if (m_UUID.size() < 38)
			XYZ_LOG_WARN("ID ", id, " is probably not UUID");
	}
	GUID::GUID(const GUID& other)
		:
		m_UUID(other.m_UUID)
	{
	}
}

#endif