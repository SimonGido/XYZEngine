#pragma once
#include "XYZ/Editor/Inspector/InspectorEditable.h"
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {
	class SpriteRendererInspector : public InspectorEditable
	{
	public:
		SpriteRendererInspector();

		virtual bool OnEditorRender(Ref<EditorRenderer> renderer) override;


		SceneEntity m_Context;
	private:
		void selectSubTextureDialog();
		void selectMaterialDialog();

	private:
		std::function<void()> m_Dialog;
		bool				  m_DialogOpen;
	};
}