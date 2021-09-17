#include "stdafx.h"
#include "Animatable.h"

namespace XYZ {

	Animatable::Animatable(SceneEntity entity)
		:
		m_SceneEntity(entity)
	{
		SetReference();
	}
	void Animatable::SetReference()
	{
		XYZ_ASSERT(m_SceneEntity.IsValid(), "Invalid entity");
		
	}
}