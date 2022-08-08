#pragma once
#include "Editor/Inspectable/Inspectable.h"

#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {
	namespace Editor {
		class PointLight2DInspector : public Inspectable
		{
		public:
			PointLight2DInspector();

			virtual bool OnEditorRender() override;
			virtual void SetSceneEntity(const SceneEntity& entity) override;
			virtual Type GetType() const override { return Inspectable::Type::Entity; }
		private:
			SceneEntity m_Context;
		};

		class SpotLight2DInspector : public Inspectable
		{
		public:
			SpotLight2DInspector();

			virtual bool OnEditorRender() override;
			virtual void SetSceneEntity(const SceneEntity& entity) override;

			virtual Type GetType() const override { return Inspectable::Type::Entity; }
		private:
			SceneEntity m_Context;
		};
	}
}