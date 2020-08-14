#pragma once
#include <bitset>

namespace XYZ {
#define MAX_ENTITIES 65536
#define MAX_COMPONENTS 64
#define NULL_ENTITY -1

	using Signature = std::bitset<MAX_COMPONENTS>;
	using ComponentType = uint16_t;

}