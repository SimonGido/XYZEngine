#pragma once
#include "XYZ/Editor/Inspector/InspectorEditable.h"
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {
	class SpriteRendererInspector : public InspectorEditable
	{
	public:
		SpriteRendererInspector();

		virtual bool OnEditorRender() override;
		virtual void SetSceneEntity(const SceneEntity& entity) override;

		virtual Type GetType() const override { return InspectorEditable::Type::Entity; }
	private:
		void selectSubTextureDialog();
		void selectMaterialDialog();

	private:
		SceneEntity			  m_Context;

		std::function<void()> m_Dialog;
		bool				  m_DialogOpen;
	};
}