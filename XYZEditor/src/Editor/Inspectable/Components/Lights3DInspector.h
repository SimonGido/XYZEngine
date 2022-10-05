#pragma once
#include "Editor/Inspectable/Inspectable.h"

#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {
	namespace Editor {
		class PointLightComponent3DInspector : public Inspectable
		{
		public:
			PointLightComponent3DInspector();

			virtual bool OnEditorRender() override;
			virtual void SetSceneEntity(const SceneEntity& entity) override { m_Context = entity; };
			virtual Type GetType() const override { return Inspectable::Type::Entity; }
		private:
			SceneEntity m_Context;
		};
	}
}