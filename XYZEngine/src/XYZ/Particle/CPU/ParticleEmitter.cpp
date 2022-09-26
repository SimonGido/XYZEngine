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



		if (Shape == EmitShape::Box)
		{
			for (uint32_t i = startId; i < endId; ++i)
			{
				generate(data, i);
				generateBox(data, i);
			}
		}
		else if (Shape == EmitShape::Circle)
		{
			for (uint32_t i = startId; i < endId; ++i)
			{
				generate(data, i);
				generateCircle(data, i);
			}
		}
		else
		{
			for (uint32_t i = startId; i < endId; ++i)
			{
				generate(data, i);
			}
		}
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
	void ParticleEmitter::generate(ParticlePool& data, uint32_t id) const
	{
		data.Wake(id);
		data.Particles[id].Color = Color;
		data.Particles[id].TexOffset = glm::vec2(0.0f, 0.0f);
		data.Particles[id].Size = Size;
		data.Particles[id].Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		data.Particles[id].LifeRemaining = LifeTime;
		data.Particles[id].Velocity = glm::linearRand(MinVelocity, MaxVelocity);
		data.Particles[id].Position = glm::vec3(0.0f);
		data.Particles[id].LightColor = LightColor;
		data.Particles[id].LightIntensity = LightIntensity;
		data.Particles[id].LightRadius = LightRadius;
	}
	void ParticleEmitter::generateBox(ParticlePool& data, uint32_t id) const
	{
		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_real_distribution<double> dist(-1.0, 1.0); // distribution in range [1, 6]

		data.Particles[id].Position = glm::linearRand(BoxMin, BoxMax);
	}
	void ParticleEmitter::generateCircle(ParticlePool& data, uint32_t id) const
	{
		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);
		
		float r = Radius * (float)sqrt(dist(rng));
		const float theta = dist(rng) * 2.0f * glm::pi<float>();
		const glm::vec2 point(
			Radius * cos(theta),
			Radius * sin(theta)
		);
		data.Particles[id].Position = glm::vec3(point.x, point.y, 0.0f);
	}
}