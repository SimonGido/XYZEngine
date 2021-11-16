#include "stdafx.h"
#include "ScriptComponentInspector.h"

#include "XYZ/Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

namespace XYZ {
	bool ScriptComponentInspector::OnEditorRender(Ref<EditorRenderer> renderer)
	{
		return EditorHelper::DrawComponent<ScriptComponent>("Script", m_Context, [&](auto& component) {
			for (const PublicField& field : component.GetFields())
			{
				EditorHelper::BeginColumns(field.GetName().c_str(), 2, 200.0f);
				std::string id = "##" + field.GetName();
				if (field.GetType() == PublicFieldType::Float)
				{
					float value = field.GetStoredValue<float>();
					if (ImGui::InputFloat(id.c_str(), &value))
					{
						field.SetStoredValue<float>(value);
					}
				}
				else if (field.GetType() == PublicFieldType::Int)
				{
					int32_t value = field.GetStoredValue<int32_t>();
					if (ImGui::InputInt(id.c_str(), &value))
					{
						field.SetStoredValue<int32_t>(value);
					}
				}
				else if (field.GetType() == PublicFieldType::UnsignedInt)
				{
					uint32_t value = field.GetStoredValue<uint32_t>();
					if (ImGui::InputInt(id.c_str(), (int*)&value))
					{
						field.SetStoredValue<uint32_t>(value);
					}
				}
				else if (field.GetType() == PublicFieldType::String)
				{
					char* value = field.GetStoredValue<char*>();
					const size_t size = strlen(value);
					if (ImGui::InputText(id.c_str(), value, size))
					{
						field.SetStoredValue<char*>(value);
					}
					delete[]value;
				}
				else if (field.GetType() == PublicFieldType::Vec2)
				{
					glm::vec2 value = field.GetStoredValue<glm::vec2>();
					if (ImGui::InputFloat2(id.c_str(), glm::value_ptr(value)))
					{
						field.SetStoredValue<glm::vec2>(value);
					}
				}
				else if (field.GetType() == PublicFieldType::Vec3)
				{
					glm::vec3 value = field.GetStoredValue<glm::vec3>();
					if (ImGui::InputFloat3(id.c_str(), glm::value_ptr(value)))
					{
						field.SetStoredValue<glm::vec3>(value);
					}
				}
				else if (field.GetType() == PublicFieldType::Vec4)
				{
					glm::vec4 value = field.GetStoredValue<glm::vec4>();
					if (ImGui::InputFloat4(id.c_str(), glm::value_ptr(value)))
					{
						field.SetStoredValue<glm::vec4>(value);
					}
				}
				EditorHelper::EndColumns();
			}
		});
	}
}