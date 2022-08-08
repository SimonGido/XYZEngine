#pragma once
#include "Editor/Inspectable/Inspectable.h"
#include "XYZ/Scene/SceneEntity.h"

#include "XYZ/Scene/Components.h"

namespace XYZ {
	namespace Editor {
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

			void acceptMaterialDragAndDrop(SpriteRenderer& component);
			void acceptSubTextureDragAndDrop(SpriteRenderer& component);

		private:
			SceneEntity			  m_Context;

			std::function<void()> m_Dialog;
			bool				  m_DialogOpen;
		};
	}
}