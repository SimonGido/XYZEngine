#include "stdafx.h"
#include "ParticleEmitter.h"

#include "XYZ/Utils/Random.h"

#include "XYZ/Debug/Profiler.h"


#include <glm/common.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>

namespace XYZ {

	EmitterBurst::EmitterBurst(uint32_t count, float time, float probability)
		:
		Count(count), Time(time), Probability(probability)
	{
	}
	ParticleEmitter::ParticleEmitter()
		:
		Shape(EmitShape::Box),
		BoxMin(-5.0f, -5.0, 0.0f),
		BoxMax(5.0f, 5.0f, 0.0f),
		Radius(7.0f),

		EmitRate(5.0f),
		LifeTime(5.0f),
		
		MinVelocity(-1.0f, -1.0f, 0.0f),
		MaxVelocity(1.0f, 1.0f, 0.0f),

		Size(1.0f),
		Color(1.0f),

		BurstInterval(5.0f),
	
		MaxLights(20),
		LightColor(1.0f),
		LightRadius(1.0f),
		LightIntensity(1.0f),

		m_EmittedParticles(0.0f),
		m_PassedTime(0.0f)
	{
	}
	void ParticleEmitter::Emit(Timestep ts, ParticlePool& data)
	{
		XYZ_PROFILE_FUNC("ParticleEmitter::Emit");
		m_EmittedParticles += EmitRate * ts;

		const uint32_t newParticles = (uint32_t)m_EmittedParticles + burstEmit();
		const uint32_t startId = data.GetAliveParticles();
		const uint32_t endId = std::min(startId + newParticles, data.GetMaxParticles());
		if (newParticles)
			m_EmittedParticles = 0.0f;


		for (uint32_t i = startId; i < endId; ++i)
		{
			data.Wake(i);
			data.Particles[i].Color = Color;
			data.Particles[i].TexOffset = glm::vec2(0.0f, 0.0f);
			data.Particles[i].Size = Size;
			data.Particles[i].Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			data.Particles[i].LifeRemaining = LifeTime;
			data.Particles[i].Velocity = glm::linearRand(MinVelocity, MaxVelocity);
			data.Particles[i].Position = glm::vec3(0.0f);
		}

		{
			XYZ_PROFILE_FUNC("ParticleSystem::Emit Lights");
			for (size_t i = startId; i < endId && m_AliveLights < MaxLights; ++i)
			{
				data.Particles[i].LightColor = LightColor;
				data.Particles[i].LightRadius = LightRadius;
				data.Particles[i].LightIntensity = LightIntensity;
				m_AliveLights++;
			}
		}
		if (Shape == EmitShape::Box)
			generateBox(data, startId, endId);
		else if (Shape == EmitShape::Circle)
			generateCircle(data, startId, endId);

		m_PassedTime += ts;
	}

	void ParticleEmitter::Kill(ParticlePool& data)
	{
		XYZ_PROFILE_FUNC("ParticleEmitter::Kill");
		uint32_t aliveParticles = data.GetAliveParticles();
		for (uint32_t i = 0; i < aliveParticles; ++i)
		{
			if (data.Particles[i].LifeRemaining <= 0.0f)
			{
				aliveParticles--;
				data.Kill(i);
				if (m_AliveLights != 0)
					m_AliveLights--;
			}
		}
	}

	uint32_t ParticleEmitter::burstEmit()
	{
		uint32_t count = 0;
		for (auto& burst : Bursts)
		{
			if (burst.Ready && burst.Time <= m_PassedTime)
			{
				if (RandomNumber(0.0f, 1.0f) <= burst.Probability)
				{
					count += burst.Count;
					burst.Ready = false;
				}
			}
		}

		if (m_PassedTime >= BurstInterval)
		{
			m_PassedTime = 0.0f;
			for (auto& burst : Bursts)
				burst.Ready = true;
		}
		return count;
	}
	void ParticleEmitter::generateBox(ParticlePool& data, uint32_t startId, uint32_t endId) const
	{
		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_real_distribution<double> dist(-1.0, 1.0); // distribution in range [1, 6]

		for (uint32_t i = startId; i < endId; i++)
		{
			data.Particles[i].Position = glm::linearRand(BoxMin, BoxMax);
		}
	}
	void ParticleEmitter::generateCircle(ParticlePool& data, uint32_t startId, uint32_t endId) const
	{
		std::random_device dev;
		std::mt19937 rng(dev());
		const std::uniform_real_distribution<float> dist(0.0f, 1.0f);

		for (uint32_t i = startId; i < endId; i++)
		{
			float r = Radius * (float)sqrt(dist(rng));
			const float theta = dist(rng) * 2.0f * glm::pi<float>();
			const glm::vec2 point(
				Radius * cos(theta),
				Radius * sin(theta)
			);
			data.Particles[i].Position = glm::vec3(point.x, point.y, 0.0f);
		}
	}
}