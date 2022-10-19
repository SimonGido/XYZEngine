#include "stdafx.h"
#include "PipelineSpecialization.h"

namespace XYZ {
	PipelineSpecializationValue::PipelineSpecializationValue(std::string name)
		: Name(std::move(name))
	{
		memset(Data.data(), 0, 4);
	}
}