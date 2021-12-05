#pragma once
#include "XYZ/Editor/Inspectable/Inspectable.h"
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {
	class SceneTagInspector : public Inspectable
	{
	public:
		SceneTagInspector();

		virtual bool OnEditorRender() override;
		virtual void SetSceneEntity(const SceneEntity& entity) override;

		virtual Type GetType() const override { return Inspectable::Type::Entity; }
	private:
		SceneEntity m_Context;
	};
}