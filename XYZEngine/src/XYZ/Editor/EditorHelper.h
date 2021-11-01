#pragma once
#include "XYZ/Scene/SceneEntity.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/perpendicular.hpp>


namespace XYZ {
	class EditorHelper
	{
	public:
		template<typename T, typename UIFunction>
		static bool DrawComponent(const std::string& name, SceneEntity entity, UIFunction uiFunction);
		template<typename T, typename UIFunction>
		static bool DrawNodeControl(const std::string& name, T& val, UIFunction uiFunction, bool& enabled);
		template<typename T, typename UIFunction, typename ConstructFunc>
		static void DrawContainerControl(const std::string& name, T& container, UIFunction uiFunction, ConstructFunc constructFunc);

		static void BeginColumns(const char* label, int count = 2, float width = 100.0f);	
		static void EndColumns(int count = 1);
		static void DrawFloatControl(const char* label, const char* dragLabel, float& value, float resetValue = 0.0f);
		static void DrawVec2Control(const std::string& label, glm::vec2& values, float resetValue = 0.0f, float columnWidth = 100.0f);	
		static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
		static void DrawVec3ControlRGB(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
		static void DrawVec4Control(const std::string& label, const char* names[4], glm::vec4& values, float resetValue = 0.0f, float columnWidth = 100.0f);
		static void DrawVec4Control2x2(const std::string& label, const char* names[4], glm::vec4& values, float resetValue = 0.0f, float columnWidth = 100.0f);
		static void DrawColorControl(const std::string& label, glm::vec4& values, float resetValue = 1.0f, float columnWidth = 100.0f);
		static void DrawSplitter(bool splitHorizontally, float thickness, float* size0, float* size1, float minSize0, float minSize1);
	

		static void PushDisabled();
		static void PopDisabled();
	};
	template<typename T, typename UIFunction>
	inline bool EditorHelper::DrawComponent(const std::string& name, SceneEntity entity, UIFunction uiFunction)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();

			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
					removeComponent = true;

				ImGui::EndPopup();
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent)
			{
				entity.RemoveComponent<T>();
				return true;
			}
			return false;
		}
		return false;
	}
	template<typename T, typename UIFunction>
	inline bool EditorHelper::DrawNodeControl(const std::string& name, T& val, UIFunction uiFunction, bool& enabled)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		if (!enabled)
			PushDisabled();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

		bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
		ImGui::PopStyleVar();
		if (!enabled)
			PopDisabled();

		ImGui::SameLine(contentRegionAvailable.x + lineHeight * 0.5f);
		
		ImGui::PushID(name.c_str());
		ImGui::Checkbox("##Enabled", &enabled);
		ImGui::PopID();

		if (!enabled)
			PushDisabled();
		if (open)
		{
			uiFunction(val);
			ImGui::TreePop();
		}
		if (!enabled)
			PopDisabled();

		return enabled;
	}
	template<typename T, typename UIFunction, typename ConstructFunc>
	inline void EditorHelper::DrawContainerControl(const std::string& name, T& container, UIFunction uiFunction, ConstructFunc constructFunc)
	{
		if (ImGui::TreeNodeEx(name.c_str(), 0, name.c_str()))
		{
			size_t counter = 0;
			for (auto it = container.begin(); it != container.end();)
			{
				ImGui::Text("%d", counter);
				ImGui::SameLine();
				uiFunction(*it, counter);

				ImGui::SameLine();
				std::string id = name + std::to_string(counter++);
				ImGui::PushID(id.c_str());
				if (ImGui::Button("-"))
				{
					it = container.erase(it);
				}
				else
				{
					++it;
				}
				ImGui::PopID();
			}

			if (ImGui::Button("+", ImVec2(40.0f, 25.0f)))
				container.push_back(constructFunc());

			ImGui::TreePop();
		}
	}
}