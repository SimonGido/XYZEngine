#include "stdafx.h"
#include "ParticleModule.h"

#include "XYZ/Renderer/SceneRenderer.h"
#include "XYZ/Scene/Components.h"

namespace XYZ {

	ParticleThreadPass::ParticleThreadPass(uint32_t maxParticles)
		:
		InstanceCount(0),
		ParticlesAlive(0),
		EmittedParticles(0.0f)
	{
		RenderData.resize(maxParticles);
		ParticlePool.resize(maxParticles);
	}


	EmissionModule::EmissionModule()
		:
		RateOverTime(2.0f),
		Enabled(true)
	{
	}
	void EmissionModule::Process(ParticleThreadPass& pass, float timeStep)
	{
		if (Enabled)
		{
			pass.EmittedParticles += RateOverTime * timeStep;
		}
	}

	VelocityOverLifeModule::VelocityOverLifeModule()
		:
		Velocity(1.0f),
		Enabled(true)
	{
	}

	void VelocityOverLifeModule::Process(ParticleCPU& particle, float timeStep)
	{
		if (Enabled)
		{
			particle.Velocity += Velocity * timeStep;
		}
	}

	//LightModule::LightModule()
	//	:
	//	Enabled(true)
	//{
	//}
	//
	//
	//void LightModule::Process(const ParticleCPU& particle, const glm::mat4& transform)
	//{
	//	if (Enabled && LightEntity.IsValid() && LightEntity.HasComponent<PointLight2D>())
	//	{
	//		SceneRenderer::SubmitLight(&LightEntity.GetComponent<PointLight2D>(), transform * glm::translate(particle.Position));
	//	}
	//}
}