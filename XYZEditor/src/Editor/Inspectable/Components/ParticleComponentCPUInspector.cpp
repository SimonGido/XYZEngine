#include "stdafx.h"
#include "ParticleComponentCPUInspector.h"

#include "Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Renderer/Renderer2D.h"

namespace XYZ {

	ParticleComponentCPUInspector::ParticleComponentCPUInspector()
		:
		Inspectable("ParticleComponentCPUInspector")
	{
	}
	bool ParticleComponentCPUInspector::OnEditorRender()
	{
		return EditorHelper::DrawComponent<ParticleComponentCPU>("Particle Component CPU", m_Context, [&](auto& component) {

			const float columnWidth = 200.0f;
			/*
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

			if (ImGui::Button("Reset"))
				system.Reset();

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

			bool physicsEnabled = moduleData->m_PhysicsModule.IsEnabled();
			EditorHelper::DrawNodeControl("Physics Module", moduleData->m_PhysicsModule, [&](auto& val) {
				bool reset = false;
				if (ImGui::Button("Shape"))
					ImGui::OpenPopup("Shape Type");
				if (ImGui::BeginPopup("Shape Type"))
				{
					if (ImGui::MenuItem("Circle"))
					{
						reset = true;
						val.m_Shape = PhysicsModule::Shape::Circle;
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::MenuItem("Box"))
					{
						reset = true;
						val.m_Shape = PhysicsModule::Shape::Box;
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}
				EditorHelper::BeginColumns("Box Size");
				reset |= ImGui::DragFloat2("##BoxSize", glm::value_ptr(val.m_BoxSize));
				EditorHelper::EndColumns();

				EditorHelper::BeginColumns("Radius");
				reset |= ImGui::DragFloat("##Radius", &val.m_Radius, 0.05f);
				EditorHelper::EndColumns();

				EditorHelper::BeginColumns("Density");
				reset |= ImGui::DragFloat("##Density", &val.m_Density, 0.05f);
				EditorHelper::EndColumns();

				EditorHelper::BeginColumns("Friction");
				reset |= ImGui::DragFloat("##Friction", &val.m_Friction, 0.05f);
				EditorHelper::EndColumns();

				EditorHelper::BeginColumns("Restitution");
				reset |= ImGui::DragFloat("##Restitution", &val.m_Restitution, 0.05f);
				EditorHelper::EndColumns();

				if (reset)
					val.Reset();

			}, physicsEnabled);
			moduleData->m_PhysicsModule.SetEnabled(physicsEnabled);
			if (physicsEnabled)
				renderColliders(renderer, moduleData.As());


			bool enabledEmitter = true;
			EditorHelper::DrawNodeControl("Emitter", moduleData->m_Emitter, [=](auto& val) {

				EditorHelper::BeginColumns("Emit rate", 2, 100.0f);
				ImGui::DragFloat("##EmitRate", &val.m_EmitRate, 0.1f);
				EditorHelper::EndColumns();
				BurstEmitter& burstEmitter = val.m_BurstEmitter;
				EditorHelper::DrawContainerControl("Bursts", burstEmitter.m_Bursts, [](BurstEmitter::Burst& burst, size_t index) {
													   const std::string indexStr = std::to_string(index);
													   const std::string burstCountID = "##BurstCount" + indexStr;
													   const std::string timeID = "##Time" + indexStr;
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
			*/
		});
	}
	void ParticleComponentCPUInspector::SetSceneEntity(const SceneEntity& entity)
	{
		m_Context = entity;
	}

	void ParticleComponentCPUInspector::renderColliders(Ref<Renderer2D>& renderer, const ParticleSystemCPU::ModuleData& moduleData)
	{
		const auto& transform = m_Context.GetComponent<TransformComponent>().WorldTransform;
		auto [translation, rotation, scale] = m_Context.GetComponent<TransformComponent>().GetWorldComponents();
		//const auto& particles = moduleData.m_Particles;
		//const auto& physicsModule = moduleData.m_PhysicsModule;
		//if (physicsModule.m_Shape == PhysicsModule::Shape::Circle)
		//{
		//	for (uint32_t i = 0; i < particles.GetAliveParticles(); ++i)
		//	{
		//		glm::vec4 position = transform * glm::vec4(particles.m_Particle[i].Position, 1.0f);
		//		renderer->SubmitCircle(position, physicsModule.m_Radius, 20, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		//	}
		//}
		//else
		//{
		//	for (uint32_t i = 0; i < particles.GetAliveParticles(); ++i)
		//	{
		//		glm::vec3 boxPos = translation + particles.m_Particle[i].Position - glm::vec3(physicsModule.m_BoxSize, 0.0f);
		//		renderer->SubmitRect(boxPos, 2.0f * physicsModule.m_BoxSize, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		//	}
		//}
	}

}