#include "stdafx.h"
#include "Component.h"


namespace XYZ {
    uint16_t Counter::getNextID()
	{
		static uint16_t nextType = 0;
		return nextType++;
	}
}