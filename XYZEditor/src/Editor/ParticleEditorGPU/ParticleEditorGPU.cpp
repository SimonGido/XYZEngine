#include "stdafx.h"
#include "ParticleEditorGPU.h"

#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"

namespace XYZ {
	namespace Editor {
		ParticleEditorGPU::ParticleEditorGPU(std::string name)
			:
			EditorPanel(name),
			m_NodeEditor("Particle Editor")
		{
			createDefaultFunctions();

			m_NodeEditor.OnStart();
			m_NodeEditor.OnBackgroundMenu = [this]() {

				for (auto& [name, func] : m_Functions)
				{
					if (ImGui::MenuItem(name.c_str()))
					{
						auto funcNode = m_NodeEditor.AddNode<ImGuiFunctionNode>(name);
						funcNode->AddOutput(func.OutputType);
						funcNode->AddOutput(func.OutputType);
						funcNode->AddOutput(func.OutputType);
						funcNode->AddOutput(func.OutputType);
						funcNode->AddOutput(func.OutputType);
						funcNode->AddOutput(func.OutputType);
						funcNode->AddOutput(func.OutputType);
						funcNode->AddOutput(func.OutputType);
						funcNode->AddOutput(func.OutputType);
						funcNode->AddOutput(func.OutputType);
						funcNode->AddOutput(func.OutputType);
						for (auto& arg : func.InputArguments)
						{
							funcNode->AddInputArgument(arg.Name, arg.Type);
						}
					}
				}
			};
		}
		ParticleEditorGPU::~ParticleEditorGPU()
		{
			m_NodeEditor.OnStop();
		}
		void ParticleEditorGPU::OnImGuiRender(bool& open)
		{
			m_NodeEditor.OnUpdate(m_Timestep);
		}
		void ParticleEditorGPU::OnUpdate(Timestep ts)
		{
			m_Timestep = ts;
		}
		bool ParticleEditorGPU::OnEvent(Event& e)
		{
			return false;
		}
		void ParticleEditorGPU::SetSceneContext(const Ref<Scene>& scene)
		{
			m_Scene = scene;

			SceneEntity particleEntity = m_Scene->GetEntityByName("Particle GPU Test0");

			auto& particleComponent = particleEntity.GetComponent<ParticleComponentGPU>();

			auto& outputLayout = particleComponent.System->GetOutputLayout();
			auto& inputLayout = particleComponent.System->GetInputLayout();


			ImGuiValueNode* outputNode = m_NodeEditor.AddNode<ImGuiValueNode>(outputLayout.GetName());
			for (auto& variable : outputLayout.GetVariables())
			{
				outputNode->AddValue(variable.Name, variable.Type);
			}

			ImGuiValueNode* inputNode = m_NodeEditor.AddNode<ImGuiValueNode>(inputLayout.GetName());
			for (auto& variable : inputLayout.GetVariables())
			{
				inputNode->AddValue(variable.Name, variable.Type);
			}
		}
		void ParticleEditorGPU::createDefaultFunctions()
		{
			{
				ParticleEditorFunction func;
				func.Name = "Vec4ToVec3";
				func.InputArguments.push_back({ VariableType::Vec4, "inputasdasdasdsdsdsd" });
				func.InputArguments.push_back({ VariableType::Vec4, "inputasdsdsd" });
				func.InputArguments.push_back({ VariableType::Vec4, "inputasdasasdsdsdsd" });
				func.InputArguments.push_back({ VariableType::Vec4, "inputasdasdasdsdsdsfghfgh" });

				func.OutputType = VariableType::Vec3;
				func.SourceCode = 
					"vec3 result = input.xyz;\n"
					"return result;";

				m_Functions[func.Name] = std::move(func);
			}
		}
	}
}