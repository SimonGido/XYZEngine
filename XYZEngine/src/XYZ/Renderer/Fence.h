#pragma once
#include "XYZ/Core/Ref.h"

namespace XYZ {

	class Fence : public RefCount
	{
	public:
		static Ref<Fence> Create(uint64_t timeOut);
	};
}