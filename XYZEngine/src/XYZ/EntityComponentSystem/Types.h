#pragma once
#include <bitset>

namespace XYZ {
	namespace ECS {
	#define MAX_ENTITIES 65536
	#define MAX_COMPONENTS 128
	#define NULL_ENTITY MAX_ENTITIES + 1
	#define HAS_GROUP_BIT 0

		using Signature = std::bitset<MAX_COMPONENTS>;
	}
}