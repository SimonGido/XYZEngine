#pragma once

#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/IG/IG.h"

#include "XYZ/BasicUI/BasicUI.h"

namespace XYZ {

	class InspectorPanel
	{
	public:
		InspectorPanel();
		~InspectorPanel();

		void SetContext(SceneEntity context);

		void OnUpdate();
	private:
		void setTransformComponent();
		void setSpriteRenderer();
		void setScriptComponent();
		void buildScriptComponent();

	private:
		bUILayout m_TransformLayout;
		bUILayout m_SpriteRendererLayout;
		bUILayout m_ScriptLayout;

		SceneEntity m_Context;
	};
}