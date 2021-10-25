#include "stdafx.h"
#include "ParticleGenerator.h"


#include <glm/common.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>

namespace XYZ {

	ParticleShapeGenerator::ParticleShapeGenerator()
		:
		m_Shape(EmitShape::Circle),
		m_BoxMin(-1.0f),
		m_BoxMax(1.0f),
		m_Radius(7.0f),
		m_Enabled(true)
	{
	}

	ParticleShapeGenerator::ParticleShapeGenerator(const ParticleShapeGenerator& other)
	{
		m_BoxMax  = other.m_BoxMax;
		m_BoxMin  = other.m_BoxMin;
		m_Shape   = other.m_Shape;
		m_Radius  = other.m_Radius;
		m_Enabled = other.m_Enabled;
	}

	ParticleShapeGenerator& ParticleShapeGenerator::operator=(const ParticleShapeGenerator& other)
	{
		m_BoxMax  = other.m_BoxMax;
		m_BoxMin  = other.m_BoxMin;
		m_Shape   = other.m_Shape;
		m_Radius  = other.m_Radius;
		m_Enabled = other.m_Enabled;
		return *this;
	}

	void ParticleShapeGenerator::Generate(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const
	{
		if (m_Enabled)
		{
			if (m_Shape == EmitShape::Box)
				generateBox(data, startId, endId);
			else if (m_Shape == EmitShape::Circle)
				generateCircle(data, startId, endId);
		}
	}
	

	void ParticleShapeGenerator::generateBox(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const
	{
		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_real_distribution<double> dist(-1.0, 1.0); // distribution in range [1, 6]

		endId = std::min(endId, data->GetMaxParticles());
		for (uint32_t i = startId; i < endId; i++)
		{
			data->m_Particle[i].Color = glm::linearRand(glm::vec4(0.0f), glm::vec4(1.0f));
			data->m_Particle[i].Position = glm::linearRand(m_BoxMin, m_BoxMax);
			data->m_TexCoord[i] = glm::vec4(0.5f, 0.5f, 0.75f, 0.75f);
			data->m_Size[i] = glm::vec2(0.5f);
		}
	}

	void ParticleShapeGenerator::generateCircle(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const
	{
		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);

		for (uint32_t i = startId; i < endId; i++)
		{
			float r = m_Radius * (float)sqrt(dist(rng));
			float theta = dist(rng) * 2.0f * glm::pi<float>();
			glm::vec2 point(
				m_Radius * cos(theta),
				m_Radius * sin(theta)
			);
			data->m_Particle[i].Position = glm::vec3(point.x, point.y, 0.0f);
			data->m_Particle[i].Color = glm::vec4(glm::linearRand(0.0f, 1.0f));
			data->m_TexCoord[i] = glm::vec4(0.5f, 0.5f, 0.75f, 0.75f);
			data->m_Size[i] = glm::vec2(0.5f);
		}
	}


	ParticleLifeGenerator::ParticleLifeGenerator()
		:
		m_LifeTime(5.0f),
		m_Enabled(true)
	{
	}

	ParticleLifeGenerator::ParticleLifeGenerator(const ParticleLifeGenerator& other)
	{
		m_LifeTime = other.m_LifeTime;
		m_Enabled  = other.m_Enabled;
	}
	ParticleLifeGenerator& ParticleLifeGenerator::operator=(const ParticleLifeGenerator& other)
	{
		m_LifeTime = other.m_LifeTime;
		m_Enabled  = other.m_Enabled;
		return *this;
	}
	void ParticleLifeGenerator::Generate(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const
	{
		if (m_Enabled)
		{
			for (uint32_t i = startId; i < endId; i++)
			{
				data->m_Particle[i].LifeRemaining = m_LifeTime;
			}
		}
	}

	ParticleRandomVelocityGenerator::ParticleRandomVelocityGenerator()
		:
		m_MinVelocity(-7.0f, -7.0f, 0.0f),
		m_MaxVelocity(7.0f, 7.0f, 0.0f),
		m_Enabled(true)
	{
	}
	ParticleRandomVelocityGenerator::ParticleRandomVelocityGenerator(const ParticleRandomVelocityGenerator& other)
	{
		m_MaxVelocity = other.m_MaxVelocity;
		m_MinVelocity = other.m_MinVelocity;
		m_Enabled = other.m_Enabled;
	}

	ParticleRandomVelocityGenerator& ParticleRandomVelocityGenerator::operator=(const ParticleRandomVelocityGenerator& other)
	{
		m_MaxVelocity = other.m_MaxVelocity;
		m_MinVelocity = other.m_MinVelocity;
		m_Enabled = other.m_Enabled;
		return *this;
	}
	void ParticleRandomVelocityGenerator::Generate(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const
	{
		for (uint32_t i = startId; i < endId; i++)
		{
			data->m_Particle[i].Velocity = glm::linearRand(m_MinVelocity, m_MaxVelocity);
		}
	}
}