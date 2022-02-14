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

				ParticleSystem& system = component.System;
				if (ImGui::Button("Reset"))
					system.Reset();

				if (ImGui::BeginTable("Particle Component Settings", 2, ImGuiTableFlags_SizingStretchProp))
				{
					int maxParticles = system.GetMaxParticles();
					UI::TableRow("Max Particles",
						[]() {ImGui::Text("Max Particles"); },
						[&]() {
						if (ImGui::InputInt("##MaxParticles", &maxParticles))
							system.SetMaxParticles(maxParticles);
					});

					float speed = system.GetSpeed();
					UI::TableRow("Simulation Speed",
						[]() {ImGui::Text("Simulation Speed"); },
						[&]() {
						if (ImGui::DragFloat("##SimulationSpeed", &speed))
							system.SetSpeed(speed);
					});

					UI::TableRow("Simulation Speed",
						[]() {ImGui::Text("Alive Particles"); },
						[&]() {ImGui::Text("%u", system.GetAliveParticles()); }
					);
					ImGui::EndTable();
				}

				ScopedLock<ParticleSystem::ModuleData> moduleData = system.GetModuleData();
				drawRotationOverLife(moduleData.As());
				drawLight(moduleData.As());
				drawTextureAnim(moduleData.As());

				drawDefaultGenerator(moduleData.As());
				drawShapeGenerator(moduleData.As());
				drawLifeGenerator(moduleData.As());
				drawRandomVelGenerator(moduleData.As()); 

				drawEmitter(moduleData.As());
			});
		}
		void ParticleComponentInspector::SetSceneEntity(const SceneEntity& entity)
		{
			m_Context = entity;
			m_SelectedBurstIndex = sc_InvalidIndex;
		}

		void ParticleComponentInspector::drawRotationOverLife(ParticleSystem::ModuleData& moduleData)
		{
			EditorHelper::DrawNodeControl("Rotation Over Life", moduleData.RotationOverLife, [](auto& val) {

				if (ImGui::BeginTable("TextureAnimationTable", 2, ImGuiTableFlags_SizingStretchProp))
				{
					UI::TableRow("EulerAngles",
						[]() {ImGui::Text("Euler Angles"); },
						[&]() { ImGui::DragFloat3("##EulerAngles", glm::value_ptr(val.EulerAngles)); }
					);

					UI::TableRow("Cycle Length",
						[]() {ImGui::Text("Cycle Length"); },
						[&]() { ImGui::DragFloat("##CycleLength", &val.CycleLength); }
					);

					ImGui::EndTable();
				}

			}, moduleData.RotationOverLife.Enabled);
		}
		void ParticleComponentInspector::drawLight(ParticleSystem::ModuleData& moduleData)
		{
			EditorHelper::DrawNodeControl("Light", moduleData.Light, [](auto& val) {

				int maxLights = val.MaxLights;
				if (ImGui::BeginTable("LightTable", 2, ImGuiTableFlags_SizingStretchProp))
				{
					UI::TableRow("Light Color",
						[]() {ImGui::Text("Light Color"); },
						[&]() { ImGui::ColorEdit3("##LightColor", glm::value_ptr(val.Light.Color)); }
					);

					UI::TableRow("Light Radius",
						[]() {ImGui::Text("Light Radius"); },
						[&]() { ImGui::DragFloat("##LightRadius", &val.Light.Radius); }
					);

					UI::TableRow("Light Intensity",
						[]() {ImGui::Text("Light Intensity"); },
						[&]() { ImGui::DragFloat("##LightIntensity", &val.Light.Intensity); }
					);

					UI::TableRow("Max Lights",
						[]() {ImGui::Text("Max Lights"); },
						[&]() { ImGui::InputInt("##MaxLights", (int*)&val.MaxLights); }
					);
					ImGui::EndTable();
				}

			}, moduleData.Light.Enabled);

		}
		void ParticleComponentInspector::drawTextureAnim(ParticleSystem::ModuleData& moduleData)
		{
			EditorHelper::DrawNodeControl("Texture Animation", moduleData.TextureAnim, [](auto& val) {

				if (ImGui::BeginTable("TextureAnimationTable", 2, ImGuiTableFlags_SizingStretchProp))
				{
					UI::TableRow("Tiles",
						[]() {ImGui::Text("Tiles"); },
						[&]() { ImGui::InputInt2("##Tiles", (int*)&val.Tiles); }
					);

					UI::TableRow("StartFrame",
						[]() {ImGui::Text("Start Frame"); },
						[&]() { ImGui::InputInt("##StartFrame", (int*)&val.StartFrame); }
					);

					UI::TableRow("Cycle Length",
						[]() {ImGui::Text("Cycle Length"); },
						[&]() { ImGui::InputFloat("##CycleLength", &val.CycleLength); }
					);

					ImGui::EndTable();
				}

			}, moduleData.TextureAnim.Enabled);

		}
		void ParticleComponentInspector::drawDefaultGenerator(ParticleSystem::ModuleData& moduleData)
		{		
			EditorHelper::DrawNodeControl("Main", moduleData.Main, [=](auto& gen) {
				if (ImGui::BeginTable("Main", 2, ImGuiTableFlags_SizingStretchProp))
				{
					UI::TableRow("Color",
						[]() {ImGui::Text("Color"); },
						[&]() { ImGui::ColorEdit4("##Color", glm::value_ptr(gen.Color)); }
					);
					UI::TableRow("Size",
						[]() {ImGui::Text("Size"); },
						[&]() { ImGui::DragFloat2("##Size", glm::value_ptr(gen.Size)); }
					);
					ImGui::EndTable();
				}
			}, moduleData.Main.Enabled);
		}
		void ParticleComponentInspector::drawShapeGenerator(ParticleSystem::ModuleData& moduleData)
		{
			EditorHelper::DrawNodeControl("Shape", moduleData.Shape, [=](auto& gen) {
				if (ImGui::BeginTable("Shape", 2, ImGuiTableFlags_SizingStretchProp))
				{
					UI::TableRow("Shape",
						[]() { ImGui::Text("Shape"); },
						[&]() 
					{ 
						const char* text = ShapeToText(gen.Shape);
						UI::ScopedTableColumnAutoWidth width(1);
						if (ImGui::Button(text))
							ImGui::OpenPopup("Shape Type");
						if (ImGui::BeginPopup("Shape Type"))
						{
							if (ImGui::MenuItem("Box"))
								gen.Shape = EmitShape::Box;
							if (ImGui::MenuItem("Circle"))
								gen.Shape = EmitShape::Circle;
							ImGui::EndPopup();
						}
					});

					UI::TableRow("Box Min",
						[]() {ImGui::Text("Box Min"); },
						[&]() { ImGui::DragFloat3("##BoxMin", glm::value_ptr(gen.BoxMin)); }
					);
					UI::TableRow("Box Max",
						[]() {ImGui::Text("Box Max"); },
						[&]() { ImGui::DragFloat3("##BoxMax", glm::value_ptr(gen.BoxMax)); }
					);
					UI::TableRow("Radius",
						[]() {ImGui::Text("Radius"); },
						[&]() { ImGui::DragFloat("##Radius", &gen.Radius); }
					);
					ImGui::EndTable();
				}
			}, moduleData.Shape.Enabled);
		}
		void ParticleComponentInspector::drawLifeGenerator(ParticleSystem::ModuleData& moduleData)
		{
			EditorHelper::DrawNodeControl("Life", moduleData.Life, [=](auto& gen) {

				if (ImGui::BeginTable("Life", 2, ImGuiTableFlags_SizingStretchProp))
				{
					UI::TableRow("Emit Rate",
						[]() {ImGui::Text("Life Time"); },
						[&]() { ImGui::DragFloat("##LifeTime", &gen.LifeTime); }
					);
	
					ImGui::EndTable();
				}
			}, moduleData.Life.Enabled);
		}
		void ParticleComponentInspector::drawRandomVelGenerator(ParticleSystem::ModuleData& moduleData)
		{
			EditorHelper::DrawNodeControl("Random Velocity", moduleData.RandomVelocity, [=](auto& gen) {

				if (ImGui::BeginTable("Random Velocity", 2, ImGuiTableFlags_SizingStretchProp))
				{
					UI::TableRow("MinVelocity",
						[]() {ImGui::Text("Min Velocity"); },
						[&]() { ImGui::DragFloat3("##MinVelocity", glm::value_ptr(gen.MinVelocity)); }
					);
					UI::TableRow("MaxVelocity",
						[]() {ImGui::Text("Max Velocity"); },
						[&]() { ImGui::DragFloat3("##MaxVelocity", glm::value_ptr(gen.MaxVelocity)); }
					);

					ImGui::EndTable();
				}

			}, moduleData.RandomVelocity.Enabled);
		}
		void ParticleComponentInspector::drawEmitter(ParticleSystem::ModuleData& moduleData)
		{
			auto& colors = EditorLayer::GetData().Color;
			
			bool enabledEmitter = true;
			EditorHelper::DrawNodeControl("Emitter", moduleData.Emitter, [=](auto& val) {
				UI::ScopedID id("Emitter");

				if (ImGui::BeginTable("Emitter", 2, ImGuiTableFlags_SizingStretchProp))
				{
					UI::TableRow("Emit Rate",
						[]() {ImGui::Text("Emit Rate"); },
						[&]() { ImGui::DragFloat("##EmitRate", &val.EmitRate); }
					);
					UI::TableRow("Burst Interval",
						[]() {ImGui::Text("Burst Interval"); },
						[&]() { ImGui::DragFloat("##BurstInterval", &val.BurstInterval); }
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
								ImGui::DragFloat(timeID.c_str(), &burst.Time);
								selected |= ImGui::IsItemDeactivated(); },
							[&]() {
								UI::ScopedTableColumnAutoWidth width(1);
								ImGui::DragFloat(probID.c_str(), &burst.Probability, 0.0f, 0.0f, 1.0f);
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
	}
}