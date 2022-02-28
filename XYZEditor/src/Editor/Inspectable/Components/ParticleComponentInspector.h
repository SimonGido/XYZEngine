#pragma once
#include "Editor/Inspectable/Inspectable.h"

#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/Particle/CPU/ParticleSystem.h"


namespace XYZ {
	namespace Editor {
		class ParticleComponentInspector : public Inspectable
		{
		public:
			ParticleComponentInspector();

			virtual bool OnEditorRender() override;
			virtual void SetSceneEntity(const SceneEntity& entity) override;

			virtual Type GetType() const override { return Inspectable::Type::Entity; }

		private:
			void drawEmitter(ParticleEmitter& emitter);

		private:
			static constexpr uint32_t sc_InvalidIndex = UINT32_MAX;

			SceneEntity m_Context;
			uint32_t	m_SelectedBurstIndex = sc_InvalidIndex;
			static constexpr float sc_VSpeed = 0.1f;
		};

		class ParticleRendererInspector : public Inspectable
		{
		public:
			ParticleRendererInspector();

			virtual bool OnEditorRender() override;
			virtual void SetSceneEntity(const SceneEntity& entity) override;

			virtual Type GetType() const override { return Inspectable::Type::Entity; }
		private:
			void acceptMaterialDragAndDrop(ParticleRenderer& component);
		
		private:
			SceneEntity m_Context;
		};
	}
}