#include "stdafx.h"
#include "ParticleComponentCPUInspector.h"

#include "XYZ/Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Renderer/EditorRenderer.h"

namespace XYZ {
	ParticleComponentCPUInspector::ParticleComponentCPUInspector()
	{
		
	}
	bool ParticleComponentCPUInspector::OnEditorRender(Ref<EditorRenderer> renderer)
	{
		return EditorHelper::DrawComponent<ParticleComponentCPU>("Particle Component CPU", m_Context, [&](auto& component) {
			
			const float columnWidth = 200.0f;

			ParticleSystemCPU& system = component.System;
			
			EditorHelper::BeginColumns("Max Particles", 2, columnWidth);
			int maxParticles = system.GetMaxParticles();
			if (ImGui::InputInt("##MaxParticles", &maxParticles))
				system.SetMaxParticles(maxParticles);
			EditorHelper::EndColumns();
		
			EditorHelper::BeginColumns("Simulation Speed", 2, columnWidth);
			float speed = system.GetSpeed();		
			if (ImGui::DragFloat("##SimulationSpeed", &speed, 0.05f))
				system.SetSpeed(speed);
			EditorHelper::EndColumns();

			EditorHelper::BeginColumns("Alive Particles", 2, columnWidth);
			ImGui::Text("%d", system.GetAliveParticles());
			EditorHelper::EndColumns();


			ScopedLock<ParticleSystemCPU::ModuleData> moduleData = system.GetModuleData();

			bool mainModuleEnabled = moduleData->m_MainModule.IsEnabled();
			EditorHelper::DrawNodeControl("Main Module", moduleData->m_MainModule, [](auto& val) {

			}, mainModuleEnabled);
			moduleData->m_MainModule.SetEnabled(mainModuleEnabled);


			bool lightModuleEnabled = moduleData->m_LightModule.IsEnabled();
			EditorHelper::DrawNodeControl("Light Module", moduleData->m_LightModule, [](auto& val) {
				
				int maxLights = val.m_MaxLights;
				EditorHelper::BeginColumns("Max Lights");
				if (ImGui::InputInt("##MaxLights", &maxLights))
					val.m_MaxLights = maxLights;
				EditorHelper::EndColumns();

			}, lightModuleEnabled);
			moduleData->m_LightModule.SetEnabled(lightModuleEnabled);


			bool textureAnimatorEnabled = moduleData->m_TextureAnimModule.IsEnabled();
			EditorHelper::DrawNodeControl("Texture Animation", moduleData->m_TextureAnimModule, [](auto& val) {

				EditorHelper::BeginColumns("Tiles");
				ImGui::InputInt2("##Tiles", (int*)&val.m_Tiles);
				EditorHelper::EndColumns();

				EditorHelper::BeginColumns("Start Frame");
				ImGui::InputInt("##StartFrame", (int*)&val.m_StartFrame);
				EditorHelper::EndColumns();

				EditorHelper::BeginColumns("Cycle Length");
				ImGui::InputFloat("##CycleLength", &val.m_CycleLength);
				EditorHelper::EndColumns();

			}, textureAnimatorEnabled);
			moduleData->m_TextureAnimModule.SetEnabled(textureAnimatorEnabled);


			bool rotationOverLifeEnabled = moduleData->m_RotationOverLife.IsEnabled();
			EditorHelper::DrawNodeControl("Rotation Over Life", moduleData->m_RotationOverLife, [](auto& val) {

				EditorHelper::BeginColumns("Euler Angles");
				ImGui::DragFloat3("##EulerAngles", glm::value_ptr(val.m_EulerAngles));
				EditorHelper::EndColumns();

				EditorHelper::BeginColumns("Cycle Length");
				ImGui::InputFloat("##CycleLength", &val.m_CycleLength);
				EditorHelper::EndColumns();

			}, rotationOverLifeEnabled);
			moduleData->m_RotationOverLife.SetEnabled(rotationOverLifeEnabled);

			bool collisionEnabled = moduleData->m_CollisionModule.IsEnabled();
			EditorHelper::DrawNodeControl("Collision Module", moduleData->m_CollisionModule, [](auto& val) {

			
			}, collisionEnabled);
			moduleData->m_CollisionModule.SetEnabled(collisionEnabled);


			bool enabledEmitter = true;
			ScopedLock<ParticleEmitterCPU> emitter = system.GetEmitter();
			EditorHelper::DrawNodeControl("Emitter", emitter.As(), [=](auto& val) {
				
				EditorHelper::BeginColumns("Emit rate", 2, 100.0f);
				ImGui::DragFloat("##EmitRate", &val.m_EmitRate, 1.0f);
				EditorHelper::EndColumns();
				BurstEmitter& burstEmitter = val.m_BurstEmitter;
				EditorHelper::DrawContainerControl("Bursts", burstEmitter.m_Bursts, [](BurstEmitter::Burst& burst, size_t index) {
					std::string indexStr = std::to_string(index);
					std::string burstCountID = "##BurstCount" + indexStr;
					std::string timeID = "##Time" + indexStr;
					ImGui::Text("Count:");
					ImGui::SameLine();			
					int count = burst.m_Count;
					if (ImGui::DragInt(burstCountID.c_str(), &count))
						burst.m_Count = count;
					ImGui::SameLine();
					ImGui::Text("Time:");
					ImGui::SameLine();
					ImGui::DragFloat(timeID.c_str(), &burst.m_Time);
				}, []() {
					return BurstEmitter::Burst(0, 0.0f);
				});
			}, enabledEmitter);
		});
	}
}