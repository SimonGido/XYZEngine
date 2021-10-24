#pragma once
#include "XYZ/Editor/Inspector/InspectorEditable.h"
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {

	class PointLight2DInspector : public InspectorEditable
	{
	public:
		virtual bool OnEditorRender(Ref<EditorRenderer> renderer) override;


		SceneEntity m_Context;
	};

	class SpotLight2DInspector : public InspectorEditable
	{
	public:
		virtual bool OnEditorRender(Ref<EditorRenderer> renderer) override;


		SceneEntity m_Context;
	};
}