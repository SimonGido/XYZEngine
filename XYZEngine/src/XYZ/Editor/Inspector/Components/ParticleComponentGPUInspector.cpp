#include "stdafx.h"
#include "ParticleComponentGPUInspector.h"

#include "XYZ/Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

namespace XYZ {
	bool ParticleComponentGPUInspector::OnEditorRender(Ref<Renderer2D> renderer)
	{
		return EditorHelper::DrawComponent<ParticleComponentGPU>("Particle Component", m_Context, [&](auto& component) {

			auto& material = component.System->m_Renderer->m_ParticleMaterial;
			auto& shader = material->GetComputeShader();
			for (auto& uniform : shader->GetVSUniformList().Uniforms)
			{
				EditorHelper::BeginColumns(uniform.GetName().c_str(), 2, 200.0f);
				std::string id = "##" + uniform.GetName();
				if (uniform.GetDataType() == ShaderUniformDataType::Int)
				{
					ImGui::InputInt(id.c_str(), &material->Get<int32_t>(uniform.GetName()));
				}
				else if (uniform.GetDataType() == ShaderUniformDataType::UInt)
				{
					ImGui::InputInt(id.c_str(), &material->Get<int32_t>(uniform.GetName()));
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
				EditorHelper::EndColumns();
			}
		});
	}
}