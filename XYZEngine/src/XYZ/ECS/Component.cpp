#include "stdafx.h"
#include "Component.h"


namespace XYZ {
    uint8_t Counter::getNextID()
	{
		static uint8_t nextType = 0;
		return nextType++;
	}
}