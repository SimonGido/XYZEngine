#pragma once
#include "XYZ/Editor/Inspector/InspectorEditable.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Particle/CPU/ParticleSystemCPU.h"

namespace XYZ {
	class ParticleComponentCPUInspector : public InspectorEditable
	{
	public:
		ParticleComponentCPUInspector();

		virtual bool OnEditorRender() override;
		virtual void SetSceneEntity(const SceneEntity& entity) override;

		virtual Type GetType() const override { return InspectorEditable::Type::Entity; }
	private:
		void renderColliders(Ref<Renderer2D>& renderer, const ParticleSystemCPU::ModuleData& moduleData);
		
	private:
		SceneEntity m_Context;
	};
}