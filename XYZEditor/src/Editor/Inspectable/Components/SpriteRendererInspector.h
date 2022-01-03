#pragma once
#include "Editor/Inspectable/Inspectable.h"
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {
	class SpriteRendererInspector : public Inspectable
	{
	public:
		SpriteRendererInspector();

		virtual bool OnEditorRender() override;
		virtual void SetSceneEntity(const SceneEntity& entity) override;

		virtual Type GetType() const override { return Inspectable::Type::Entity; }
	private:
		void selectSubTextureDialog();
		void selectMaterialDialog();

	private:
		SceneEntity			  m_Context;

		std::function<void()> m_Dialog;
		bool				  m_DialogOpen;
	};
}