#include "stdafx.h"
#include "ImGuiVariableTypeExtension.h"
#include "XYZ/Scene/Blueprint.h"

#include "ImGui.h"

#include <imgui.h>

#define MAX_PATH 260

namespace XYZ {
	
	ImGuiVariableTypeExtension::ImGuiVariableTypeExtension()
	{
		RegisterEditFunction("BufferType", [](const char* id, std::byte* data) {
			
			BlueprintBufferType& type = (BlueprintBufferType&)*data;
			const std::string bufferTypeString = BufferTypeToString(type);
			const std::string inputID = "##" + bufferTypeString;
			
			ImGui::InputText(inputID.c_str(), UI::Utils::GetPathBuffer(bufferTypeString), MAX_PATH, ImGuiInputTextFlags_ReadOnly);
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			{
				if (type == BlueprintBufferType::Uniform)
					type = BlueprintBufferType::Storage;
				else
					type = BlueprintBufferType::Uniform;
			}
			
			return false;
		});

		RegisterEditFunction("float", [](const char*id, std::byte* data) {
			return ImGui::InputFloat(id, (float*)data);
		});

		RegisterEditFunction("vec2", [](const char* id, std::byte* data) {
			return ImGui::InputFloat2(id, (float*)data);
			});

		RegisterEditFunction("vec3", [](const char* id, std::byte* data) {
			return ImGui::InputFloat3(id, (float*)data);
			});

		RegisterEditFunction("vec4", [](const char* id, std::byte* data) {
			return ImGui::InputFloat4(id, (float*)data);
			});

		RegisterEditFunction("uint", [](const char* id, std::byte* data) {
			return ImGui::InputInt(id, (int*)data);
			});

		RegisterEditFunction("int", [](const char* id, std::byte* data) {
			return ImGui::InputInt(id, (int*)data);
			});

		RegisterEditFunction("ivec2", [](const char* id, std::byte* data) {
			return ImGui::InputInt2(id, (int*)data);
			});

		RegisterEditFunction("ivec3", [](const char* id, std::byte* data) {
			return ImGui::InputInt3(id, (int*)data);
			});

		RegisterEditFunction("ivec4", [](const char* id, std::byte* data) {
			return ImGui::InputInt4(id, (int*)data);
			});

		RegisterEditFunction("bool", [](const char* id, std::byte* data) {
			return ImGui::Checkbox(id, (bool*)data);
			});

		RegisterEditFunction("mat4", [](const char* id, std::byte* data) {

			const std::string id0 = id;
			const std::string id1 = id0 + "1";
			const std::string id2 = id0 + "2";
			const std::string id3 = id0 + "3";
			
			bool result = false;
			result |= ImGui::InputFloat4(id0.c_str(), (float*)data);
			result |= ImGui::InputFloat4(id1.c_str(), (float*)&data[16]);
			result |= ImGui::InputFloat4(id2.c_str(), (float*)&data[32]);
			result |= ImGui::InputFloat4(id3.c_str(), (float*)&data[48]);

			return result;
		});
		
	}
	void ImGuiVariableTypeExtension::RegisterEditFunction(const std::string& name, EditFunction func)
	{
		XYZ_ASSERT(m_EditFunctions.find(name) == m_EditFunctions.end(), "");
		m_EditFunctions[name] = std::move(func);
	}

	bool ImGuiVariableTypeExtension::EditValue(const std::string& name, const char*id, std::byte* data)
	{
		auto it = m_EditFunctions.find(name);
		if (it == m_EditFunctions.end())
			return false;

		return it->second(id, data);
	}

}