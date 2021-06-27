#include "stdafx.h"
#include "ParticleComponentGPUInspector.h"

#include "XYZ/Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

namespace XYZ {
	bool ParticleComponentGPUInspector::OnEditorRender()
	{
		return EditorHelper::DrawComponent<ParticleComponentGPU>("Particle Component", m_Context, [&](auto& component) {

			auto& material = component.System->m_Renderer.ParticleMaterial;
			auto& shader = material->GetComputeShader();
			for (auto& uniform : shader->GetVSUniformList().Uniforms)
			{
				EditorHelper::BeginColumns(uniform.Name.c_str(), 2, 200.0f);
				std::string id = "##" + uniform.Name;
				if (uniform.DataType == UniformDataType::Int)
				{
					ImGui::InputInt(id.c_str(), &material->Get<int32_t>(uniform.Name));
				}
				else if (uniform.DataType == UniformDataType::UInt)
				{
					ImGui::InputInt(id.c_str(), &material->Get<int32_t>(uniform.Name));
				}
				else if (uniform.DataType == UniformDataType::Float)
				{
					ImGui::InputFloat(id.c_str(), &material->Get<float>(uniform.Name));
				}
				else if (uniform.DataType == UniformDataType::Vec2)
				{
					ImGui::InputFloat2(id.c_str(), glm::value_ptr(material->Get<glm::vec2>(uniform.Name)));
				}
				else if (uniform.DataType == UniformDataType::Vec3)
				{
					ImGui::InputFloat3(id.c_str(), glm::value_ptr(material->Get<glm::vec3>(uniform.Name)));
				}
				else if (uniform.DataType == UniformDataType::Vec4)
				{
					ImGui::InputFloat4(id.c_str(), glm::value_ptr(material->Get<glm::vec4>(uniform.Name)));
				}
				EditorHelper::EndColumns();
			}
		});
	}
}