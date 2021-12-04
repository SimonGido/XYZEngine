#pragma once
#include "XYZ/Editor/Inspector/InspectorEditable.h"
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {
	class TransformInspector : public InspectorEditable
	{
	public:
		TransformInspector();

		virtual bool OnEditorRender() override;
		virtual void SetSceneEntity(const SceneEntity& entity) override;

		virtual Type GetType() const override { return InspectorEditable::Type::Entity; }
	private:
		SceneEntity m_Context;
	};
}