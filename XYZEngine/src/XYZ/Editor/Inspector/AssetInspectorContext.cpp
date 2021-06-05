#include "stdafx.h"
#include "AssetInspectorContext.h"

#include <imgui.h>
#include <imgui_internal.h>


#include <glm/gtc/type_ptr.hpp>

namespace XYZ {
	namespace Helper {
		static void BeginColumns(const char* label, int count = 2, float width = 100.0f)
		{
			ImGui::Columns(count);
			ImGui::PushID(label);
			ImGui::SetColumnWidth(0, width);
			ImGui::Text(label);
			ImGui::NextColumn();

		}
		static void EndColumns()
		{
			ImGui::PopID();
			ImGui::Columns(1);
		}
	}
	namespace Editor {
		void AssetInspectorContext::OnImGuiRender()
		{
			if (m_Context.Raw())
			{
				Helper::BeginColumns("Name");
				ImGui::Text(m_Context->FileName.c_str());		
				Helper::EndColumns();
				Helper::BeginColumns("File Path");
				ImGui::Text(m_Context->FilePath.c_str());
				Helper::EndColumns();
				ImGui::NewLine();

				if (m_Context->Type == AssetType::Material)
					drawMaterial(m_Context.As<Material>());
			}
		}
		void AssetInspectorContext::SetContext(const Ref<Asset>& context)
		{
			m_Context = context;
		}
		void AssetInspectorContext::drawMaterial(Ref<Material>& material)
		{
			ImGui::Text("Values:");
			for (auto& uniform : material->GetShader()->GetFSUniformList().Uniforms)
			{
				std::string id = "##" + uniform.Name;
				Helper::BeginColumns(uniform.Name.c_str());
				ImGui::PushItemWidth(ImGui::CalcItemWidth());
				if (uniform.DataType == UniformDataType::Int)
				{
					ImGui::InputInt(id.c_str(), material->Get<int>(uniform.Name));					
				}
				else if (uniform.DataType == UniformDataType::IntVec2)
				{			
					ImGui::InputInt2(id.c_str(), glm::value_ptr(*material->Get<glm::ivec2>(uniform.Name)));
				}
				else if (uniform.DataType == UniformDataType::IntVec3)
				{
					ImGui::InputInt3(id.c_str(), glm::value_ptr(*material->Get<glm::ivec3>(uniform.Name)));
				}
				else if (uniform.DataType == UniformDataType::IntVec4)
				{
					ImGui::InputInt4(id.c_str(), glm::value_ptr(*material->Get<glm::ivec4>(uniform.Name)));
				}
				else if (uniform.DataType == UniformDataType::Float)
				{
					ImGui::InputFloat(id.c_str(), material->Get<float>(uniform.Name));
				}
				else if (uniform.DataType == UniformDataType::Vec2)
				{
					ImGui::InputFloat2(id.c_str(), glm::value_ptr(*material->Get<glm::vec2>(uniform.Name)));
				}
				else if (uniform.DataType == UniformDataType::Vec3)
				{				
					ImGui::InputFloat3(id.c_str(), glm::value_ptr(*material->Get<glm::vec3>(uniform.Name)));
				}
				else if (uniform.DataType == UniformDataType::Vec4)
				{	
					ImGui::InputFloat4(id.c_str(), glm::value_ptr(*material->Get<glm::vec4>(uniform.Name)));		
				}
				ImGui::PopItemWidth();
				Helper::EndColumns();
			}
			
		}
	}
}