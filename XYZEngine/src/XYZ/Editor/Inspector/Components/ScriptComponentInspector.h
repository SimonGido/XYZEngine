#pragma once
#include "XYZ/Editor/Inspector/InspectorEditable.h"
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {
	class ScriptComponentInspector : public InspectorEditable
	{
	public:
		virtual bool OnEditorRender() override;


		SceneEntity m_Context;
	};
}