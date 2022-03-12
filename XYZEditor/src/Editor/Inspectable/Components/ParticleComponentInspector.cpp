#include "stdafx.h"
#include "ParticleComponentInspector.h"

#include "Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/ImGui/ImGui.h"

#include "EditorLayer.h"

namespace XYZ {
	namespace Editor {

		const char* ShapeToText(EmitShape shape)
		{
			switch (shape)
			{
			case EmitShape::None: return "None";
			case EmitShape::Box: return "Box";
			case EmitShape::Circle: return "Circle";
			}
			return nullptr;
		}

		ParticleComponentInspector::ParticleComponentInspector()
			:
			Inspectable("ParticleComponentInspector")
		{
		}
		bool ParticleComponentInspector::OnEditorRender()
		{
			return EditorHelper::DrawComponent<ParticleComponent>("Particle Component ", m_Context, [&](auto& component) {

				const float columnWidth = 200.0f;
				
				if (ImGui::Button("Reset"))
					component.System->Reset();
				
				if (ImGui::BeginTable("Particle Component Settings", 2, ImGuiTableFlags_SizingStretchProp))
				{
					int maxParticles = component.System->GetMaxParticles();
					UI::TableRow("Max Particles",
						[]() {ImGui::Text("Max Particles"); },
						[&]() {
						if (ImGui::InputInt("##MaxParticles", &maxParticles))
							component.System->SetMaxParticles(maxParticles);
					});
				
					UI::TableRow("Simulation Speed",
						[]() {ImGui::Text("Simulation Speed"); },
						[&]() {
						ImGui::DragFloat("##SimulationSpeed", &component.System->Speed, sc_VSpeed);
					});
				
					UI::TableRow("Simulation Speed",
						[]() {ImGui::Text("Alive Particles"); },
						[&]() {ImGui::Text("%u", component.System->GetAliveParticles()); }
					);

					ImGui::EndTable();
				}
	
				EditorHelper::DrawNodeControl("Animation", component, [](auto& component) {
					if (ImGui::BeginTable("Animation table", 2, ImGuiTableFlags_SizingStretchProp))
					{
						UI::TableRow("Animation Tiles",
							[]() {ImGui::Text("Tiles"); },
							[&]() { ImGui::InputInt2("##AnimationTiles", (int*)&component.System->AnimationTiles); }
						);

						UI::TableRow("AnimationStartFrame",
							[]() {ImGui::Text("Start Frame"); },
							[&]() { ImGui::InputInt("##AnimationStartFrame", (int*)&component.System->AnimationStartFrame); }
						);

						UI::TableRow("Animation Cycle Length",
							[]() {ImGui::Text("Cycle Length"); },
							[&]() { ImGui::InputFloat("##AnimationCycleLength", &component.System->AnimationCycleLength); }
						);
						ImGui::EndTable();
					}
				}, component.System->ModuleEnabled[ParticleSystem::TextureAnimation]);


				EditorHelper::DrawNodeControl("Rotation Over Life", component, [](auto& component) {
					if (ImGui::BeginTable("Rotation table", 2, ImGuiTableFlags_SizingStretchProp))
					{
						UI::TableRow("Rotation",
							[]() {ImGui::Text("Rotation"); },
							[&]() { ImGui::DragFloat3("##RotationDrag", glm::value_ptr(component.System->EndSize)); }
						);

						ImGui::EndTable();
					}
					}, component.System->ModuleEnabled[ParticleSystem::RotationOverLife]);

				EditorHelper::DrawNodeControl("Size Over Life", component, [](auto& component) {
					if (ImGui::BeginTable("Size table", 2, ImGuiTableFlags_SizingStretchProp))
					{
						UI::TableRow("Size",
							[]() {ImGui::Text("Size"); },
							[&]() { ImGui::DragFloat3("##SizeDrag", glm::value_ptr(component.System->EndSize)); }
						);

						ImGui::EndTable();
					}
					}, component.System->ModuleEnabled[ParticleSystem::SizeOverLife]);


				EditorHelper::DrawNodeControl("Color Over Life", component, [](auto& component) {
					if (ImGui::BeginTable("Color table", 2, ImGuiTableFlags_SizingStretchProp))
					{
						UI::TableRow("Color",
							[]() {ImGui::Text("Color"); },
							[&]() { ImGui::DragFloat3("##ColorDrag", glm::value_ptr(component.System->EndColor)); }
						);

						ImGui::EndTable();
					}
					}, component.System->ModuleEnabled[ParticleSystem::ColorOverLife]);
				
				EditorHelper::DrawNodeControl("Light Over Life", component, [](auto& component) {
					if (ImGui::BeginTable("Lights table", 2, ImGuiTableFlags_SizingStretchProp))
					{
						UI::TableRow("End Color",
							[]() {ImGui::Text("End Color"); },
							[&]() { ImGui::DragFloat3("##EndColorDrag", glm::value_ptr(component.System->LightEndColor)); }
						);
						UI::TableRow("Light End Intensity",
							[]() {ImGui::Text("End Intensity"); },
							[&]() { ImGui::DragFloat("##", &component.System->LightEndIntensity); }
						);
						UI::TableRow("Light End Radius",
							[]() {ImGui::Text("End Radius"); },
							[&]() { ImGui::DragFloat("##", &component.System->LightEndRadius); }
						);
						ImGui::EndTable();
					}
					}, component.System->ModuleEnabled[ParticleSystem::LightOverLife]);

				drawEmitter(component.System->Emitter);
			});
		}
		void ParticleComponentInspector::SetSceneEntity(const SceneEntity& entity)
		{
			m_Context = entity;
			m_SelectedBurstIndex = sc_InvalidIndex;
		}

		void ParticleComponentInspector::drawEmitter(ParticleEmitter& emitter)
		{
			auto& colors = EditorLayer::GetData().Color;
			
			bool enabledEmitter = true;
			EditorHelper::DrawNodeControl("Emitter", emitter, [=](auto& val) {
				UI::ScopedID id("Emitter");
			
				if (ImGui::BeginTable("Emitter", 2, ImGuiTableFlags_SizingStretchProp))
				{
					////////////////////////
					UI::TableRow("Shape",
						[]() { ImGui::Text("Shape"); },
						[&]()
					{
						const char* text = ShapeToText(val.Shape);
						UI::ScopedTableColumnAutoWidth width(1);
						if (ImGui::Button(text))
							ImGui::OpenPopup("Shape Type");
						if (ImGui::BeginPopup("Shape Type"))
						{
							if (ImGui::MenuItem("None"))
								val.Shape = EmitShape::None;
							if (ImGui::MenuItem("Box"))
								val.Shape = EmitShape::Box;
							if (ImGui::MenuItem("Circle"))
								val.Shape = EmitShape::Circle;
							ImGui::EndPopup();
						}
					});
			
					UI::TableRow("Box Min",
						[]() {ImGui::Text("Box Min"); },
						[&]() { ImGui::DragFloat3("##BoxMin", glm::value_ptr(val.BoxMin), sc_VSpeed); }
					);
					UI::TableRow("Box Max",
						[]() {ImGui::Text("Box Max"); },
						[&]() { ImGui::DragFloat3("##BoxMax", glm::value_ptr(val.BoxMax), sc_VSpeed); }
					);
					UI::TableRow("Radius",
						[]() {ImGui::Text("Radius"); },
						[&]() { ImGui::DragFloat("##Radius", &val.Radius, sc_VSpeed); }
					);
			
					////////////////////////
					UI::TableRow("Emit Rate",
						[]() {ImGui::Text("Emit Rate"); },
						[&]() { ImGui::DragFloat("##EmitRate", &val.EmitRate, sc_VSpeed); }
					);
					UI::TableRow("Life Time",
						[]() {ImGui::Text("Life Time"); },
						[&]() { ImGui::DragFloat("##LifeTime", &val.LifeTime, sc_VSpeed); }
					);
			
			
					////////////////////////				
					UI::TableRow("MinVelocity",
						[]() {ImGui::Text("Min Velocity"); },
						[&]() { ImGui::DragFloat3("##MinVelocity", glm::value_ptr(val.MinVelocity), sc_VSpeed); }
					);
					UI::TableRow("MaxVelocity",
						[]() {ImGui::Text("Max Velocity"); },
						[&]() { ImGui::DragFloat3("##MaxVelocity", glm::value_ptr(val.MaxVelocity), sc_VSpeed); }
					);
					UI::TableRow("Size",
						[]() {ImGui::Text("Size"); },
						[&]() { ImGui::DragFloat2("##Size", glm::value_ptr(val.Size), sc_VSpeed); }
					);
					UI::TableRow("Color",
						[]() {ImGui::Text("Color"); },
						[&]() { ImGui::ColorEdit4("##Color", glm::value_ptr(val.Color), sc_VSpeed); }
					);
				
					////////////////////////
					UI::TableRow("Burst Interval",
						[]() {ImGui::Text("Burst Interval"); },
						[&]() { ImGui::DragFloat("##BurstInterval", &val.BurstInterval, sc_VSpeed); }
					);

					UI::TableRow("Max Lights",
						[]() {ImGui::Text("Max Lights"); },
						[&]() { ImGui::DragInt("##MaxLights", (int*)&val.MaxLights); }
					);

					UI::TableRow("Light Color",
						[]() {ImGui::Text("Light Color"); },
						[&]() { ImGui::ColorEdit3("##LightColor", glm::value_ptr(val.LightColor), sc_VSpeed); }
					);

					UI::TableRow("Light Radius",
						[]() {ImGui::Text("Light Radius"); },
						[&]() { ImGui::DragFloat("##LightRadius", &val.LightRadius, sc_VSpeed); }
					);

					UI::TableRow("Light Intensity",
						[]() {ImGui::Text("Light Intensity"); },
						[&]() { ImGui::DragFloat("##LightIntensity", &val.LightIntensity, sc_VSpeed); }
					);
					ImGui::EndTable();
				}
				
				ImGui::Text("Bursts");
				bool newSelected = false;
				auto& bursts = val.Bursts;
				if (ImGui::BeginTable("Bursts", 3, ImGuiTableFlags_SizingStretchSame))
				{
					ImGui::TableSetupColumn("Count");
					ImGui::TableSetupColumn("Time");
					ImGui::TableSetupColumn("Probability");
					ImGui::TableHeadersRow();
			
					uint32_t index = 0;
					for (auto& burst : bursts)
					{
						const std::string indexStr = std::to_string(index);
						const std::string countID = "##Count" + indexStr;
						const std::string timeID = "##Time" + indexStr;
						const std::string probID = "##Probability" + indexStr;
			
						UI::ScopedColorStack color(m_SelectedBurstIndex == index,
							ImGuiCol_FrameBg, colors[ED::ContainerSelectedItem]);
			
						bool selected = false;
						UI::TableRow(indexStr.c_str(),
							[&]() {
								UI::ScopedTableColumnAutoWidth width(1);
								ImGui::DragInt(countID.c_str(), (int*)&burst.Count);
								selected = ImGui::IsItemDeactivated(); },
							[&]() {
								UI::ScopedTableColumnAutoWidth width(1);
								ImGui::DragFloat(timeID.c_str(), &burst.Time, sc_VSpeed);
								selected |= ImGui::IsItemDeactivated(); },
							[&]() {
								UI::ScopedTableColumnAutoWidth width(1);
								ImGui::DragFloat(probID.c_str(), &burst.Probability, sc_VSpeed, 0.0f, 1.0f);
								selected |= ImGui::IsItemDeactivated(); }
							);
						
					
						if (selected)
						{
							m_SelectedBurstIndex = index;
							newSelected = true;
						}
						index++;
					}
					ImGui::EndTable();
				}
			
				UI::ScopedStyleStack style(true, ImGuiStyleVar_ItemSpacing, glm::vec2(0.0f));
				if (ImGui::Button("+"))
					bursts.push_back({});
				ImGui::SameLine();
				if (ImGui::Button("-"))
				{
					if (m_SelectedBurstIndex != sc_InvalidIndex)
					{
						bursts.erase(bursts.begin() + m_SelectedBurstIndex);
						m_SelectedBurstIndex = sc_InvalidIndex;
					}
					else if (!bursts.empty())
					{
						bursts.pop_back();
					}
				}
				
				if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !newSelected)
					m_SelectedBurstIndex = sc_InvalidIndex;
			
			}, enabledEmitter);
		}
		ParticleRendererInspector::ParticleRendererInspector()
			:
			Inspectable("Particle Renderer")
		{
		}
		bool ParticleRendererInspector::OnEditorRender()
		{
			return EditorHelper::DrawComponent<ParticleRenderer>("Particle Renderer ", m_Context, [&](auto& component) {
				UI::ScopedStyleStack style(true, ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 5.0f });
				UI::ScopedColorStack color(true,
					ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f },
					ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f },
					ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f }
				);
				const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;


				if (ImGui::BeginTable("##ParticleRendererTable", 2, ImGuiTableFlags_SizingStretchProp))
				{
					std::string materialName = "None";
					if (component.MaterialAsset.Raw() && AssetManager::Exist(component.MaterialAsset->GetHandle()))
					{
						materialName = Utils::GetFilename(AssetManager::GetMetadata(component.MaterialAsset).FilePath.string());
					}
					UI::TableRow("Material",
						[]() { ImGui::Text("Material"); },
						[&]() {
						UI::ScopedWidth w(150.0f);
						ImGui::InputText("##Material", (char*)materialName.c_str(), materialName.size(), ImGuiInputTextFlags_ReadOnly);
						acceptMaterialDragAndDrop(component);
					});

					ImGui::EndTable();
				}

			});
		}
		void ParticleRendererInspector::SetSceneEntity(const SceneEntity& entity)
		{
			m_Context = entity;
		}
		void ParticleRendererInspector::acceptMaterialDragAndDrop(ParticleRenderer& component)
		{
			char* materialAssetPath = nullptr;
			if (UI::DragDropTarget("AssetDragAndDrop", &materialAssetPath))
			{
				std::filesystem::path path(materialAssetPath);
				if (AssetManager::Exist(path))
				{
					auto& metadata = AssetManager::GetMetadata(path);
					if (metadata.Type == AssetType::Material)
					{
						Ref<MaterialAsset> materialAsset = AssetManager::GetAsset<MaterialAsset>(metadata.Handle);
						component.MaterialAsset = materialAsset;
					}
				}
			}
		}
}
}