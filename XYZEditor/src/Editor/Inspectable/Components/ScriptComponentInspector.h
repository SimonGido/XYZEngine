#pragma once
#include "Editor/Inspectable/Inspectable.h"
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {
	class ScriptComponentInspector : public Inspectable
	{
	public:
		ScriptComponentInspector();

		virtual bool OnEditorRender() override;
		virtual void SetSceneEntity(const SceneEntity& entity) override;

		virtual Type GetType() const override { return Inspectable::Type::Entity; }
	private:
		SceneEntity m_Context;
	};
}