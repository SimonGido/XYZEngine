#pragma once
#include <bitset>

namespace XYZ {
	namespace ECS {
	#define INVALID_COMPONENT_TYPE 0
	#define MAX_ENTITIES 65536
	#define MAX_COMPONENTS 128
	#define NULL_ENTITY MAX_ENTITIES + 1

		using Signature = std::bitset<MAX_COMPONENTS>;
	}
}