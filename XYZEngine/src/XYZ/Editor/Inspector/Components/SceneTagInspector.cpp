#include "stdafx.h"
#include "Lights2DInspector.h"

#include "XYZ/Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"
#include "SceneTagInspector.h"


namespace XYZ {
	bool SceneTagInspector::OnEditorRender(Ref<Renderer2D> renderer)
	{
		return EditorHelper::DrawComponent<SceneTagComponent>("Scene Tag", m_Context, [&](auto& component) {

			std::string& tag = m_Context.GetComponent<SceneTagComponent>().Name;
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			std::strncpy(buffer, tag.c_str(), sizeof(buffer));
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		});
	}
}