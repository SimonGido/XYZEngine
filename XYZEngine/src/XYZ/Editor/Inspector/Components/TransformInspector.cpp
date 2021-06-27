#include "stdafx.h"
#include "TransformInspector.h"

#include "XYZ/Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

namespace XYZ {
	bool TransformInspector::OnEditorRender()
	{
		return EditorHelper::DrawComponent<TransformComponent>("Transform", m_Context, [&](auto& component) {

			EditorHelper::DrawVec3Control("Translation", component.Translation);
			glm::vec3 rotation = glm::degrees(component.Rotation);
			EditorHelper::DrawVec3Control("Rotation", rotation);
			component.Rotation = glm::radians(rotation);
			EditorHelper::DrawVec3Control("Scale", component.Scale, 1.0f);
		});
	}
}
