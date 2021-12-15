#include "stdafx.h"
#include "MaterialInspector.h"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>


namespace XYZ {
	namespace Editor {
		MaterialInspector::MaterialInspector()
			:
			Inspectable("MaterialInspector")
		{
		}
		bool MaterialInspector::OnEditorRender()
		{
			ImGui::Text("Values:");
			
			const Ref<Shader>& shader = m_MaterialAsset->GetShader();
			for (const auto& [buffName, buffer] : shader->GetBuffers())
			{
				for (const auto& [uniName, uniform] : buffer.Uniforms)
				{
					std::string id = "##" + buffName + uniform.GetName();
					if (uniform.GetDataType() == ShaderUniformDataType::Int)
					{
						ImGui::InputInt(id.c_str(), &m_MaterialAsset->GetInt(uniform.GetName()));
					}
					else if (uniform.GetDataType() == ShaderUniformDataType::IntVec2)
					{
						ImGui::InputInt2(id.c_str(), glm::value_ptr(m_MaterialAsset->GetIVector2(uniform.GetName())));
					}
					else if (uniform.GetDataType() == ShaderUniformDataType::IntVec3)
					{
						ImGui::InputInt3(id.c_str(), glm::value_ptr(m_MaterialAsset->GetIVector3(uniform.GetName())));
					}
					else if (uniform.GetDataType() == ShaderUniformDataType::IntVec4)
					{
						ImGui::InputInt4(id.c_str(), glm::value_ptr(m_MaterialAsset->GetIVector4(uniform.GetName())));
					}
					else if (uniform.GetDataType() == ShaderUniformDataType::Float)
					{
						ImGui::InputFloat(id.c_str(), &m_MaterialAsset->GetFloat(uniform.GetName()));
					}
					else if (uniform.GetDataType() == ShaderUniformDataType::Vec2)
					{
						ImGui::InputFloat2(id.c_str(), glm::value_ptr(m_MaterialAsset->GetVector2(uniform.GetName())));
					}
					else if (uniform.GetDataType() == ShaderUniformDataType::Vec3)
					{
						ImGui::InputFloat3(id.c_str(), glm::value_ptr(m_MaterialAsset->GetVector3(uniform.GetName())));
					}
					else if (uniform.GetDataType() == ShaderUniformDataType::Vec4)
					{
						ImGui::InputFloat4(id.c_str(), glm::value_ptr(m_MaterialAsset->GetVector4(uniform.GetName())));
					}
				}
			}
			return false;
		}
		void MaterialInspector::SetAsset(const Ref<Asset>& asset)
		{
			//XYZ_ASSERT(asset->Type == AssetType::Material, "Asset must have AssetType Material");
			//m_MaterialAsset = asset.As<Material>();
		}
	}
}