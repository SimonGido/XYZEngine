#include "stdafx.h"
#include "Animatable.h"

namespace XYZ {

	Animatable::Animatable(SceneEntity entity, const SetRefCallbackFn& setReferenceCallback)
		:
		m_SceneEntity(entity),
		m_SetRefCallback(setReferenceCallback)
	{
		SetReference();
	}
	void Animatable::SetReference()
	{
		XYZ_ASSERT(m_SceneEntity.IsValid(), "Invalid entity");
		m_SetRefCallback(m_ComponentReference, m_SceneEntity);
	}
}