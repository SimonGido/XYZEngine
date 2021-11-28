#pragma once
#include "XYZ/Editor/Inspector/InspectorEditable.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Particle/CPU/ParticleSystemCPU.h"

namespace XYZ {
	class ParticleComponentCPUInspector : public InspectorEditable
	{
	public:
		ParticleComponentCPUInspector();

		virtual bool OnEditorRender(Ref<Renderer2D> renderer) override;


		SceneEntity m_Context;
	private:
		void renderColliders(Ref<Renderer2D>& renderer, const ParticleSystemCPU::ModuleData& moduleData);
	};
}