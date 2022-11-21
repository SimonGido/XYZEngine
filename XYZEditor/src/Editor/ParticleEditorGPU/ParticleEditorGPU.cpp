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
			m_NodeEditor = std::make_shared<XYZ::UI::ImGuiNodeContext>("Blueprint Editor");
			m_NodeEditor->OnStart();
			m_NodeEditor->OnBackgroundMenu = [this]() {
				onBackgroundMenu();
			};
		}
		ParticleEditorGPU::~ParticleEditorGPU()
		{
			m_NodeEditor->OnStop();
		}
		void ParticleEditorGPU::OnImGuiRender(bool& open)
		{
			if (m_ParticleSystem.Raw())
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
							m_NodeEditor->OnUpdate(m_Timestep);
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
					type.Variables.push_back({ var.Name, var.Type});
				}
			}


			const uint32_t valueFlags =
				  XYZ::UI::ImGuiNodeValueFlags_AllowName
				| XYZ::UI::ImGuiNodeValueFlags_AllowEdit;


			XYZ::UI::ImGuiNode* outputBufferNode = m_NodeEditor->AddNode(XYZ::UI::ImGuiNodeFlags_AllowInput);
			outputBufferNode->SetName("buffer_Particles");
			outputBufferNode->SetType(m_VariableManager.GetVariable("buffer"));
			outputBufferNode->AddValue("binding", m_VariableManager.GetVariable("uint"), valueFlags);
			outputBufferNode->AddValue("set", m_VariableManager.GetVariable("uint"), valueFlags);
			auto& outputVal = outputBufferNode->AddValue("output", m_VariableManager.GetVariable(outputLayout.GetName()), 0);
			outputVal.SetArray(true);
			for (auto& variable : outputLayout.GetVariables())
			{
				outputVal.AddValue(variable.Name, variable.Type,
					XYZ::UI::ImGuiNodeValueFlags_AllowName
				  | XYZ::UI::ImGuiNodeValueFlags_AllowInput);
			}


			XYZ::UI::ImGuiNode* inputBufferNode = m_NodeEditor->AddNode(0);
			inputBufferNode->SetName("buffer_ParticleProperties");
			inputBufferNode->SetType(m_VariableManager.GetVariable("buffer"));
			inputBufferNode->AddValue("binding", m_VariableManager.GetVariable("uint"), valueFlags);
			inputBufferNode->AddValue("set", m_VariableManager.GetVariable("uint"), valueFlags);
			auto& inputVal = inputBufferNode->AddValue("input", m_VariableManager.GetVariable(inputLayout.GetName()), XYZ::UI::ImGuiNodeValueFlags_AllowName);
			inputVal.SetArray(true);
			for (auto& variable : inputLayout.GetVariables())
			{
				inputVal.AddValue(variable.Name, variable.Type, 
					  XYZ::UI::ImGuiNodeValueFlags_AllowOutput
					| XYZ::UI::ImGuiNodeValueFlags_AllowName);
			}
		}
	
		void ParticleEditorGPU::onBackgroundMenu()
		{
			const uint32_t nodeFlags =
				XYZ::UI::ImGuiNodeFlags_AllowInput
				| XYZ::UI::ImGuiNodeFlags_AllowOutput
				| XYZ::UI::ImGuiNodeFlags_AllowName;

			const uint32_t inputValueFlags =
				XYZ::UI::ImGuiNodeValueFlags_AllowInput
			  | XYZ::UI::ImGuiNodeValueFlags_AllowName;
				
			const uint32_t outputValueFlags =
				XYZ::UI::ImGuiNodeValueFlags_AllowOutput;

			for (auto& func : m_BlueprintManager.GetFunctions())
			{
				if (ImGui::MenuItem(func.Name.c_str()))
				{
					auto funcNode = m_NodeEditor->AddNode(nodeFlags);
					funcNode->SetName(func.Name);
					funcNode->SetType(m_VariableManager.GetVariable("function"));
					for (auto& arg : func.Arguments)
					{
						if (!arg.Output)
							funcNode->AddValue(arg.Name, arg.Type, inputValueFlags);
					}
					for (auto& arg : func.Arguments)
					{
						if (arg.Output)
							funcNode->AddValue(arg.Name, arg.Type, outputValueFlags);
					}
				}
			}
		}
		void ParticleEditorGPU::onParticleSystemSelected()
		{
			if (m_ParticleSystem.Raw())
			{

			}
			else
			{
				m_NodeEditor->Clear();
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
			
			auto funcSequenceNodes = m_NodeEditor->FindNodeSequence("EntryPoint");
			auto nodes = m_NodeEditor->GetNodes();

			std::unordered_map<std::string, XYZ::UI::ImGuiNode*> uniqueStructs;
			for (auto node : nodes)
			{
				if (node->GetType().Name != "function"
				&&  node->GetType().Name != "buffer")
					uniqueStructs[node->GetType().Name] = node;
			}

			// Add struct definitions
			for (auto& [typeName, node] : uniqueStructs)
			{
				BlueprintStruct blueprintStruct;
				blueprintStruct.Name = typeName;
				for (auto& value : node->GetValues())
				{
					auto& blueprintVar = blueprintStruct.Variables.emplace_back();
					blueprintVar.Name = value.GetName();
					blueprintVar.Type = value.GetType();
				}
				result->AddStruct(blueprintStruct);
			}

			// Add buffers
			for (auto node : nodes)
			{
				if (node->GetType() == m_VariableManager.GetVariable("buffer"))
				{
					BlueprintBuffer buffer;
					buffer.Name = node->GetName();
					buffer.LayoutType = BlueprintBufferLayout::STD430;
					buffer.Type = BlueprintBufferType::Storage;

					for (auto& val : node->GetValues())
					{
						if (val.GetName() == "binding")
							buffer.Binding = val.GetValue<uint32_t>();
						else if (val.GetName() == "set")
							buffer.Set = val.GetValue<uint32_t>();
						else
						{
							BlueprintVariable variable;
							variable.Name = val.GetName();
							variable.Type = val.GetType();
							variable.IsArray = val.IsArray();
							buffer.Variables.push_back(variable);
						}
					}
					result->AddBuffer(buffer);
				}
			}
		
			// Add functions
			for (auto& blueprintFunction : m_BlueprintManager.GetFunctions())
			{
				// Start from one, skip entry point
				for (size_t i = 1; i < funcSequenceNodes.size(); ++i)
				{
					auto funcNode = funcSequenceNodes[i];
					if (funcNode->GetName() == blueprintFunction.Name)
					{
						result->AddFunction(blueprintFunction);
						break;
					}
				}
			}

			
			BlueprintFunctionSequence sequence;
			if (!funcSequenceNodes.empty())
			{
				sequence.EntryPoint = *m_BlueprintManager.FindFunction(funcSequenceNodes[0]->GetName());
				for (size_t i = 1; i < funcSequenceNodes.size(); ++i)
				{
					auto funcNode = funcSequenceNodes[i];
					auto& blueprintFunction = *m_BlueprintManager.FindFunction(funcNode->GetName());
					
					auto& funcCall = sequence.FunctionCalls.emplace_back();
					funcCall.Name = blueprintFunction.Name;
					funcCall.Arguments = blueprintFunction.Arguments;
				}
				result->SetFunctionSequence(sequence);
			}
			
			result->Rebuild();
			return result;
		}
	}
}