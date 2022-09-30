#pragma once

#include "Editor/Inspectable/Inspectable.h"

#include "XYZ/Scene/SceneEntity.h"
namespace XYZ {
	namespace Editor {
		class MeshComponentInspector : public Inspectable
		{
		public:
			MeshComponentInspector();

			virtual bool OnEditorRender() override;
			virtual void SetSceneEntity(const SceneEntity& entity) override;

			virtual Type GetType() const override { return Inspectable::Type::Entity; }
		private:
			SceneEntity m_Context;
		};

		class AnimatedMeshComponentInspector : public Inspectable
		{
		public:
			AnimatedMeshComponentInspector();

			virtual bool OnEditorRender() override;
			virtual void SetSceneEntity(const SceneEntity& entity) override;

			virtual Type GetType() const override { return Inspectable::Type::Entity; }
		private:
			SceneEntity m_Context;
		};
	}
}