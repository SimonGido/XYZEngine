#pragma once
#include "XYZ/Editor/Inspector/InspectorEditable.h"
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {
	class Physics2DInspector : public InspectorEditable
	{
	public:
		virtual bool OnEditorRender() override;


		SceneEntity m_Context;

	private:
		static constexpr glm::vec4 sc_ColliderColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	};
}