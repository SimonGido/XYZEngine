#include "stdafx.h"
#include "XYZ/Core/GUID.h"

#include <Rpc.h>

namespace XYZ {

	GUID::GUID()
	{
		UUID uuid;
		UuidCreate(&uuid);

		unsigned char* str;
		UuidToStringA(&uuid, &str);

		m_UUID = (char*)str;
		RpcStringFreeA(&str);
	}
	GUID::GUID(const GUID& other)
		:
		m_UUID(other.m_UUID)
	{

	}
}