#pragma once
#include "XYZ/Editor/Inspector/InspectorEditable.h"
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {

	class PointLight2DInspector : public InspectorEditable
	{
	public:
		PointLight2DInspector();

		virtual bool OnEditorRender() override;
		virtual void SetSceneEntity(const SceneEntity& entity) override;
		virtual Type GetType() const override { return InspectorEditable::Type::Entity; }
	private:
		SceneEntity m_Context;
	};

	class SpotLight2DInspector : public InspectorEditable
	{
	public:
		SpotLight2DInspector();

		virtual bool OnEditorRender() override;
		virtual void SetSceneEntity(const SceneEntity& entity) override;

		virtual Type GetType() const override { return InspectorEditable::Type::Entity; }
	private:
		SceneEntity m_Context;
	};
}