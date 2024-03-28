#include "stdafx.h"
#include "ParticleEditorGPU.h"

#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/ImGui/ImGui.h"
#include "XYZ/Debug/Profiler.h"

#include "Editor/Event/EditorEvents.h"


namespace XYZ {
	namespace Editor {
		ParticleEditorGPU::ParticleEditorGPU(std::string name)
			:
			EditorPanel(name),
			m_BlueprintManager(&m_VariableManager)
		{
		}
		ParticleEditorGPU::~ParticleEditorGPU()
		{
		}
		void ParticleEditorGPU::OnImGuiRender(bool& open)
		{
			if (m_ParticleSystem.Valid())
			{
				if (ImGui::Begin(m_Name.c_str()))
				{
					UI::SplitterV(&m_SplitterWidth, "##ToolsPanel", "##NodeEditor",
						[&]() 
						{
							if (ImGui::Button("Compile"))
							{
								m_Blueprint = createBlueprint();
							}
							editBlueprintTypes();
						},
						[&]() 
						{
							
						});					
				}
				ImGui::End();
			}
		}
		void ParticleEditorGPU::OnUpdate(Timestep ts)
		{
			m_Timestep = ts;
		}
		bool ParticleEditorGPU::OnEvent(Event& e)
		{
			if (e.GetEventType() == EventType::Editor)
			{
				EditorEvent& editorEvent = (EditorEvent&)e;
				if (editorEvent.GetEditorEventType() == EditorEventType::AssetSelected)
				{
					AssetSelectedEvent& assetSelectedEvent = (AssetSelectedEvent&)editorEvent;
					auto asset = assetSelectedEvent.GetAsset();
					if (asset->GetAssetType() == AssetType::ParticleSystemGPU)
					{
						m_ParticleSystem = asset.As<ParticleSystemGPU>();
					}
					else
					{
						m_ParticleSystem = nullptr;
					}
					onParticleSystemSelected();
				}
			}
			return false;
		}
		void ParticleEditorGPU::SetSceneContext(const Ref<Scene>& scene)
		{
			m_Scene = scene;

			SceneEntity particleEntity = m_Scene->GetEntityByName("Particle GPU Test0");

			auto& particleComponent = particleEntity.GetComponent<ParticleComponentGPU>();

			m_ParticleSystem = particleComponent.System;

			auto& outputLayout = particleComponent.System->GetOutputLayout();
			auto& inputLayout = particleComponent.System->GetInputLayout();



			m_VariableManager.RegisterVariable(outputLayout.GetName(), outputLayout.GetStride());
			m_VariableManager.RegisterVariable(inputLayout.GetName(), inputLayout.GetStride());
			
			{
				auto& type = m_BlueprintManager.GetStructTypes().emplace_back();
				type.Name = outputLayout.GetName();
				for (auto& var : outputLayout.GetVariables())
				{
					type.Variables.push_back({ var.Name, var.Type});
				}
			}
			{
				auto& type = m_BlueprintManager.GetStructTypes().emplace_back();
				type.Name = inputLayout.GetName();
				for (auto& var : inputLayout.GetVariables())
				{
					type.Variables.push_back({ var.Name, var.Type });
				}
			}
		}
	
		void ParticleEditorGPU::onBackgroundMenu()
		{
			
			for (auto& func : m_BlueprintManager.GetFunctions())
			{
				if (ImGui::MenuItem(func.Name.c_str()))
				{
					
				}
			}
		}
		void ParticleEditorGPU::onParticleSystemSelected()
		{
			if (m_ParticleSystem.Valid())
			{

			}
			else
			{
				
			}
		}
		void ParticleEditorGPU::editBlueprintTypes()
		{
			for (auto& structType : m_BlueprintManager.GetStructTypes())
			{
				bool editType = false; // True if we want to edit type of variable
				size_t& selectedIndex = m_PerTypeSelection[structType.Name];
				const std::string popupName = "##EditVariableType" + structType.Name;

				if (ImGui::BeginPopup(popupName.c_str()))
				{
					for (const auto& [name, type] : m_VariableManager.GetVariableTypes())
					{
						if (ImGui::MenuItem(name.c_str()))
						{
							structType.Variables[selectedIndex].Type = type;
						}
					}
					ImGui::EndPopup();
				}
		
				UI::ContainerControl<2>(structType.Name.c_str(), structType.Variables, {"Type", "Name"}, selectedIndex,
					[&](BlueprintVariable& var, size_t& selectedIndex, size_t index) {

						const std::string indexStr = std::to_string(index);
			
						UI::TableRow(indexStr.c_str(),
							[&]() 
							{
								ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), var.Type.Name.c_str());
								// Type was selected
								if (UI::IsTextDeactivated())
								{
									selectedIndex = index;
									editType = true;
								}
							},
							[&]() 
							{
								UI::InputText(indexStr.c_str(), var.Name);
							});
					}, ImGuiTableFlags_SizingStretchProp);
				
				if (editType)
					ImGui::OpenPopup(popupName.c_str());		
			}			
		}
		Ref<Blueprint> ParticleEditorGPU::createBlueprint() const
		{
			Ref<Blueprint> result = Ref<Blueprint>::Create();
			
			
			return result;
		}
	}
}