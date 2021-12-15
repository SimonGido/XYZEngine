#pragma once
#include "Editor/Inspectable/Inspectable.h"

#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Particle/CPU/ParticleSystemCPU.h"

namespace XYZ {
	class ParticleComponentCPUInspector : public Inspectable
	{
	public:
		ParticleComponentCPUInspector();

		virtual bool OnEditorRender() override;
		virtual void SetSceneEntity(const SceneEntity& entity) override;

		virtual Type GetType() const override { return Inspectable::Type::Entity; }
	private:
		void renderColliders(Ref<Renderer2D>& renderer, const ParticleSystemCPU::ModuleData& moduleData);
		
	private:
		SceneEntity m_Context;
	};
}