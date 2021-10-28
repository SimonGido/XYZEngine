#pragma once
#include "XYZ/Editor/Inspector/InspectorEditable.h"
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {
	class ParticleComponentCPUInspector : public InspectorEditable
	{
	public:
		ParticleComponentCPUInspector();

		virtual bool OnEditorRender(Ref<EditorRenderer> renderer) override;


		SceneEntity m_Context;
	};
}