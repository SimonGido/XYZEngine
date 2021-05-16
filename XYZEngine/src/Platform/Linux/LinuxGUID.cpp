#include "stdafx.h"
#include "XYZ/Core/GUID.h"

#ifdef XYZ_PLATFORM_LINUX
#include <uuid/uuid.h>

namespace XYZ {

	GUID::GUID()
	{
		uuid_t uuid;
		uuid_generate_random(uuid);
		char s[37];
		uuid_unparse(uuid, s);
		// TODO: Finish
	}
}

#endif