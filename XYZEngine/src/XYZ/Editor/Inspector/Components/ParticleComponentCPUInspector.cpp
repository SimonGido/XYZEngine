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


			ScopedLock<ParticleSystemCPU::UpdateData> updateData = system.GetUpdateData();

			EditorHelper::DrawNodeControl("Main Updater", updateData->m_MainUpdater, [](auto& val) {

			}, updateData->m_MainUpdater.m_Enabled);

			EditorHelper::DrawNodeControl("Light Updater", updateData->m_LightUpdater, [](auto& val) {
				
				int maxLights = val.m_MaxLights;
				EditorHelper::BeginColumns("Max Lights");
				if (ImGui::InputInt("##MaxLights", &maxLights))
					val.m_MaxLights = maxLights;
				EditorHelper::EndColumns();

			}, updateData->m_LightUpdater.m_Enabled);

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