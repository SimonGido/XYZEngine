#include "stdafx.h"
#include "InspectorPanel.h"

#include "XYZ/Renderer/Font.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Utils/FileSystem.h"
#include "XYZ/Utils/StringUtils.h"
#include "XYZ/Script/ScriptEngine.h"
#include "XYZ/Asset/AssetManager.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Scene/Components.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>

namespace XYZ {

	namespace Editor {
		InspectorPanel::InspectorPanel()
			:
			m_Context(nullptr)
		{
		}
		void InspectorPanel::OnImGuiRender(Ref<EditorRenderer> renderer)
		{
			if (ImGui::Begin("Inspector"))
			{
				if (m_Context)
					m_Context->OnImGuiRender(renderer);
			}
			ImGui::End();		
		}

		void InspectorPanel::SetContext(InspectorContext* context)
		{
			m_Context = context;
		}
	}
}