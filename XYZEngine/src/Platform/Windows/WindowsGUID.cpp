#include "stdafx.h"
#include "XYZ/Core/GUID.h"

#include <array>

#ifdef XYZ_PLATFORM_WINDOWS
#include <Rpc.h>
#include <combaseapi.h>

#pragma comment(lib, "rpcrt4.lib")

namespace XYZ {
	
	GUID::GUID()
	{
		UUID uuid;
		auto result = CoCreateGuid(&uuid);
		m_Data1 = uuid.Data1;
		m_Data2 = uuid.Data2;
		m_Data3 = uuid.Data3;
		for (uint32_t i = 0; i < 8; ++i)
			m_Data4[i] = uuid.Data4[i];
	}
	GUID::GUID(const std::string& id)
	{
		unsigned char uuid[37];
		for (size_t i = 0; i < id.size(); ++i)
			uuid[i] = id[i];
		uuid[36] = '\0';

		UUID uid;
		auto status = UuidFromStringA(uuid, &uid);
		XYZ_ASSERT(!status, "");

		m_Data1 = uid.Data1;
		m_Data2 = uid.Data2;
		m_Data3 = uid.Data3;
		for (uint32_t i = 0; i < 8; ++i)
			m_Data4[i] = uid.Data4[i];

	}
	GUID::GUID(const GUID& other)
		:
		m_Data1(other.m_Data1),
		m_Data2(other.m_Data2),
		m_Data3(other.m_Data3)
	{
		for (uint32_t i = 0; i < 8; ++i)
			m_Data4[i] = other.m_Data4[i];
	}

	GUID& GUID::operator=(const std::string& id)
	{
		unsigned char uuid[37];
		for (size_t i = 0; i < id.size(); ++i)
			uuid[i] = id[i];
		uuid[36] = '\0';

		UUID uid;
		auto status = UuidFromStringA(uuid, &uid);
		XYZ_ASSERT(!status, "");

		m_Data1 = uid.Data1;
		m_Data2 = uid.Data2;
		m_Data3 = uid.Data3;
		for (uint32_t i = 0; i < 8; ++i)
			m_Data4[i] = uid.Data4[i];

		return *this;
	}

	GUID::operator std::string() const
	{
		RPC_CSTR szUuid = NULL;
		UUID uuid;
		uuid.Data1 = m_Data1;
		uuid.Data2 = m_Data2;
		uuid.Data3 = m_Data3;
		for (uint32_t i = 0; i < 8; ++i)
			uuid.Data4[i] = m_Data4[i];
		UuidToStringA(&uuid, &szUuid);

		return (char*)szUuid;
	}

	GUID::operator std::string()
	{
		RPC_CSTR szUuid = NULL;
		UUID uuid;
		uuid.Data1 = m_Data1;
		uuid.Data2 = m_Data2;
		uuid.Data3 = m_Data3;
		for (uint32_t i = 0; i < 8; ++i)
			uuid.Data4[i] = m_Data4[i];
		UuidToStringA(&uuid, &szUuid);

		return (char*)szUuid;
	}

}

#endif