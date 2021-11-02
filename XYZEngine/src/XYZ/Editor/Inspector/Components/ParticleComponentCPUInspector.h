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
	private:
		void renderColliders(Ref<EditorRenderer>& renderer, const ParticleSystemCPU::ModuleData& moduleData);
	};
}