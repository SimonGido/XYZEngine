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
		m_Data[1] = 0;
		UUID uuid;
		auto result = CoCreateGuid(&uuid);
		m_Data[0] = uuid.Data1 + (static_cast<uint64_t>(uuid.Data2) << 32) + (static_cast<uint64_t>(uuid.Data3) << 48);
		for (uint32_t i = 0; i < 8; ++i)
			m_Data[1] |= static_cast<uint64_t>(uuid.Data4[i]) << (i * 8);
	}
	GUID::GUID(const std::string& id)
	{
		unsigned char uuid[37];
		for (size_t i = 0; i < id.size(); ++i)
			uuid[i] = id[i];
		uuid[36] = '\0';

		UUID uid;
		const auto status = UuidFromStringA(uuid, &uid);
		XYZ_ASSERT(!status, "");

		m_Data[1] = 0;
		m_Data[0] = uid.Data1 + (static_cast<uint64_t>(uid.Data2) << 32) + (static_cast<uint64_t>(uid.Data3) << 48);
		for (uint32_t i = 0; i < 8; ++i)
			m_Data[1] |= static_cast<uint64_t>(uid.Data4[i]) << (i * 8);
	}
	GUID::GUID(const GUID& other)
	{
		m_Data[0] = other.m_Data[0];
		m_Data[1] = other.m_Data[1];
	}
 
	GUID& GUID::operator=(const std::string& id)
	{
		unsigned char uuid[37];
		for (size_t i = 0; i < id.size(); ++i)
			uuid[i] = id[i];
		uuid[36] = '\0';

		UUID uid;
		const auto status = UuidFromStringA(uuid, &uid);
		XYZ_ASSERT(!status, "");
		m_Data[1] = 0;

		m_Data[0] = uid.Data1 + (static_cast<uint64_t>(uid.Data2) << 32) + (static_cast<uint64_t>(uid.Data3) << 48);
		for (uint32_t i = 0; i < 8; ++i)
			m_Data[1] |= static_cast<uint64_t>(uid.Data4[i]) << (i * 8);

		return *this;
	}

	GUID::operator std::string() const
	{
		RPC_CSTR szUuid = NULL;
		UUID uuid;
		uuid.Data1 = static_cast<uint32_t>(m_Data[0]);
		uuid.Data2 = static_cast<uint16_t>(m_Data[0] >> 32);
		uuid.Data3 = static_cast<uint16_t>(m_Data[0] >> 48);
		for (uint32_t i = 0; i < 8; ++i)
			uuid.Data4[i] = static_cast<uint8_t>(m_Data[1] >> (i * 8));

		auto ret = UuidToStringA(&uuid, &szUuid);
		if (ret != 0)
			XYZ_ASSERT(false, "");
		return (char*)szUuid;
	}

	GUID::operator std::string()
	{
		RPC_CSTR szUuid = NULL;
		UUID uuid;
		uuid.Data1 = static_cast<uint32_t>(m_Data[0]);
		uuid.Data2 = static_cast<uint16_t>(m_Data[0] >> 32);
		uuid.Data3 = static_cast<uint16_t>(m_Data[0] >> 48);
		for (uint32_t i = 0; i < 8; ++i)
			uuid.Data4[i] = static_cast<uint8_t>(m_Data[1] >> (i * 8));
		auto ret = UuidToStringA(&uuid, &szUuid);
		if (ret != 0)
			XYZ_ASSERT(false, "");
		return (char*)szUuid;
	}
}

#endif