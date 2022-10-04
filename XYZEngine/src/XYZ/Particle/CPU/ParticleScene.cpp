#include "stdafx.h"
#include "ParticleScene.h"



namespace XYZ {
	static void Mat4ToTransformData(glm::vec4* transformRows, const glm::mat4& transform)
	{
		transformRows[0] = { transform[0][0], transform[1][0], transform[2][0], transform[3][0] };
		transformRows[1] = { transform[0][1], transform[1][1], transform[2][1], transform[3][1] };
		transformRows[2] = { transform[0][2], transform[1][2], transform[2][2], transform[3][2] };
	}

	ParticleScene::ParticleScene(uint32_t maxParticles)
		:
		m_Pool(maxParticles)
	{
		m_RenderData.InstanceData.resize(maxParticles);
	}

	void ParticleScene::Begin(Timestep ts)
	{
		m_ParticleSystems.clear();
		m_Timestep += ts; // Accumulate timestep
	}
	void ParticleScene::SubmitSystem(const glm::mat4& transform, Ref<ParticleSystem> system, ModulesEnabled enabledModules)
	{
		m_ParticleSystems.push_back({ system, std::move(enabledModules), transform });		
	}
	void ParticleScene::End()
	{
		for (auto& data : m_ParticleSystems)
		{
			if (data.Enabled[ParticleComponent::RotationOverLife])
			{
				data.System->UpdateRotation(m_Pool);
			}
			if (data.Enabled[ParticleComponent::TextureAnimation])
			{
				data.System->UpdateAnimation(m_Pool);
			}
			if (data.Enabled[ParticleComponent::ColorOverLife])
			{
				data.System->UpdateColorOverLife(m_Pool);
			}
			if (data.Enabled[ParticleComponent::SizeOverLife])
			{
				data.System->UpdateSizeOverLife(m_Pool);
			}
			if (data.Enabled[ParticleComponent::LightOverLife])
			{
				data.System->UpdateLightOverLife(m_Pool);
			}
		}
		m_Timestep = {};
	}
	void ParticleScene::EndAsync(ThreadPool& threadPool)
	{
		// We did not finish previous frame jobs, skip this update
		if (m_JobsCount != 0)
			return;
					
		m_RenderData.SystemDataIndices.resize(m_ParticleSystems.size());
		
		uint32_t aliveParticles = m_Pool.GetAliveParticles();
		for (uint32_t i = 0; i < aliveParticles; ++i)
		{
			if (m_Pool.Particles[i].LifeRemaining <= 0.0f)
			{
				aliveParticles--;
				m_Pool.Kill(i);
			}
		}
		for (uint32_t i = 0; i < m_Pool.GetAliveParticles(); ++i)
		{
			m_Pool.Particles[i].LifeRemaining -= m_Timestep.GetSeconds();
			m_Pool.Particles[i].Position += m_Pool.Particles[i].Velocity * m_Timestep.GetSeconds();
		}
		

		// Calculate initial number of jobs, update ParticleSystem and reset SystemDataIndices
		size_t index = 0;
		for (auto& data : m_ParticleSystems)
		{
			data.System->Update(m_Timestep, m_Pool);
			for (auto enabled : data.Enabled)
			{
				if (enabled)
					m_JobsCount++;
			}
			m_RenderData.SystemDataIndices[index++] = 0;
		}
		m_Timestep = {};
		
		float buildRenderDataJobs = (float)m_Pool.GetAliveParticles() / (float)sc_PerJobCount;
		m_JobsCount += std::ceil(buildRenderDataJobs);


		Ref<ParticleScene> instance = this;
		index = 0;
		for (auto& data : m_ParticleSystems)
		{
			Ref<ParticleSystem> system = data.System;

			if (data.Enabled[ParticleComponent::RotationOverLife])
			{
				threadPool.PushJob([instance, system]() mutable {
					system->UpdateRotation(instance->m_Pool);
					instance->m_JobsCount--;
				});
			}
			if (data.Enabled[ParticleComponent::TextureAnimation])
			{
				threadPool.PushJob([instance, system]() mutable {
					system->UpdateAnimation(instance->m_Pool);
					instance->m_JobsCount--;
					});
			}
			if (data.Enabled[ParticleComponent::ColorOverLife])
			{
				threadPool.PushJob([instance, system]() mutable {
					system->UpdateColorOverLife(instance->m_Pool);
					instance->m_JobsCount--;
				});
			}
			if (data.Enabled[ParticleComponent::SizeOverLife])
			{
				threadPool.PushJob([instance, system]() mutable {
					system->UpdateSizeOverLife(instance->m_Pool);
					instance->m_JobsCount--;
				});
			}
			if (data.Enabled[ParticleComponent::LightOverLife])
			{
				threadPool.PushJob([instance, system]() mutable {
					system->UpdateLightOverLife(instance->m_Pool);
					instance->m_JobsCount--;
				});
			}
			uint32_t lastEndID = 0;
			buildRenderData(threadPool, data, lastEndID);
			m_RenderData.SystemDataIndices[index++] = lastEndID;
		}
	}
	void ParticleScene::buildRenderData(ThreadPool& threadPool, SystemData& data, uint32_t& lastEndID)
	{
		Ref<ParticleScene> instance = this;

		uint32_t startID = lastEndID;
		uint32_t endID = std::min(startID + sc_PerJobCount, data.System->m_EndParticle);
		lastEndID = endID;
		if (endID != startID)
		{
			threadPool.PushJob([instance, startID, endID, tr = data.Transform]() mutable {
				instance->buildRenderData(tr, startID, endID);
				instance->m_JobsCount--;
			});

			while (endID < data.System->m_EndParticle)
			{
				startID = lastEndID;
				endID = std::min(startID + sc_PerJobCount, data.System->m_EndParticle);
				lastEndID = endID;

				threadPool.PushJob([instance, startID, endID, tr = data.Transform]() mutable {
					instance->buildRenderData(tr, startID, endID);
					instance->m_JobsCount--;
				});
			}
		}		
	}
	void ParticleScene::buildRenderData(const glm::mat4& transform, uint32_t startId, uint32_t endId)
	{
		for (uint32_t i = startId; i < endId; ++i)
		{
			const auto& particle = m_Pool.Particles[i];

			const glm::mat4 particleTransform =
				glm::translate(particle.Position)
				* glm::toMat4(particle.Rotation)
				* glm::scale(particle.Size);


			const glm::mat4 worldParticleTransform = transform * particleTransform;


			m_RenderData.InstanceData[i].Color = particle.Color;
			Mat4ToTransformData(m_RenderData.InstanceData[i].Transform, worldParticleTransform);
			m_RenderData.InstanceData[i].TexOffset = particle.TexOffset;
		}
	}
}