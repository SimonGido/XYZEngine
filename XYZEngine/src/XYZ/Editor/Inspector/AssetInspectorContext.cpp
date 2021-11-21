#include "stdafx.h"
#include "AssetInspectorContext.h"

#include "XYZ/Utils/StringUtils.h"

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
		void AssetInspectorContext::OnImGuiRender(Ref<EditorRenderer> renderer)
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

				switch (m_Context->Type)
				{
				case XYZ::AssetType::Scene:
					
					break;
				case XYZ::AssetType::Texture:
					drawTexture2D(m_Context.As<Texture2D>());
					break;
				case XYZ::AssetType::SubTexture:
					
					break;
				case XYZ::AssetType::Material:
					drawMaterial(m_Context.As<Material>());
					break;
				case XYZ::AssetType::Shader:
					drawShader(m_Context.As<Shader>());
					break;
				case XYZ::AssetType::Font:
					
					break;
				case XYZ::AssetType::Audio:
					break;
				case XYZ::AssetType::Script:
					break;
				case XYZ::AssetType::SkeletalMesh:
					break;
				case XYZ::AssetType::None:
					break;
				default:
					break;
				}
			}
		}
		void AssetInspectorContext::SetContext(const Ref<Asset>& context)
		{
			m_Context = context;
		}
		void AssetInspectorContext::drawMaterial(Ref<Material>& material)
		{
			ImGui::Text("Values:");
			/*
			for (auto& uniform : material->GetShader()->GetFSUniformList().Uniforms)
			{
				std::string id = "##" + uniform.GetName();
				Helper::BeginColumns(uniform.GetName().c_str());
				ImGui::PushItemWidth(ImGui::CalcItemWidth());
				if (uniform.GetDataType() == ShaderUniformDataType::Int)
				{
					ImGui::InputInt(id.c_str(), &material->Get<int>(uniform.GetName()));					
				}
				else if (uniform.GetDataType() == ShaderUniformDataType::IntVec2)
				{			
					ImGui::InputInt2(id.c_str(), glm::value_ptr(material->Get<glm::ivec2>(uniform.GetName())));
				}
				else if (uniform.GetDataType() == ShaderUniformDataType::IntVec3)
				{
					ImGui::InputInt3(id.c_str(), glm::value_ptr(material->Get<glm::ivec3>(uniform.GetName())));
				}
				else if (uniform.GetDataType() == ShaderUniformDataType::IntVec4)
				{
					ImGui::InputInt4(id.c_str(), glm::value_ptr(material->Get<glm::ivec4>(uniform.GetName())));
				}
				else if (uniform.GetDataType() == ShaderUniformDataType::Float)
				{
					ImGui::InputFloat(id.c_str(), &material->Get<float>(uniform.GetName()));
				}
				else if (uniform.GetDataType() == ShaderUniformDataType::Vec2)
				{
					ImGui::InputFloat2(id.c_str(), glm::value_ptr(material->Get<glm::vec2>(uniform.GetName())));
				}
				else if (uniform.GetDataType() == ShaderUniformDataType::Vec3)
				{				
					ImGui::InputFloat3(id.c_str(), glm::value_ptr(material->Get<glm::vec3>(uniform.GetName())));
				}
				else if (uniform.GetDataType() == ShaderUniformDataType::Vec4)
				{	
					ImGui::InputFloat4(id.c_str(), glm::value_ptr(material->Get<glm::vec4>(uniform.GetName())));		
				}
				ImGui::PopItemWidth();
				Helper::EndColumns();
			}
			*/
			
		}
		void AssetInspectorContext::drawShader(Ref<Shader>& shader)
		{
			if (ImGui::Button("Reload"))
				shader->Reload();
		}
		void AssetInspectorContext::drawTexture2D(Ref<Texture2D>& texture)
		{		
			Helper::BeginColumns("Image path:");
			ImGui::PushItemWidth(ImGui::CalcItemWidth());

			ImGui::Text("%s", texture->GetPath().c_str());

			ImGui::PopItemWidth();
			Helper::EndColumns();

			float panelWidth = ImGui::GetContentRegionAvail().x;
			const float aspect = (float)texture->GetHeight() / (float)texture->GetWidth();
			ImGui::Image((void*)(uint64_t)texture.Raw(), { panelWidth, panelWidth * aspect }, { 0, 1 }, { 1, 0 });
		}
	}
}