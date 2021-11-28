#pragma once
#include "XYZ/Editor/Inspector/InspectorEditable.h"
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {
	class ParticleComponentGPUInspector : public InspectorEditable
	{
	public:
		virtual bool OnEditorRender(Ref<Renderer2D> renderer) override;


		SceneEntity m_Context;
	};
}