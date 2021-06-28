#include "stdafx.h"
#include "ParticleGenerator.h"


#include <glm/common.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>

namespace XYZ {
	ParticleGenerator::ParticleGenerator()
	{
	}



	ParticleShapeGenerator::ParticleShapeGenerator()
		:
		m_Shape(EmitShape::Box),
		m_BoxMin(-1.0f),
		m_BoxMax(1.0f),
		m_Radius(1.0f)
	{
	}

	ParticleShapeGenerator::ParticleShapeGenerator(const ParticleShapeGenerator& other)
	{
		std::scoped_lock lock(other.m_Mutex);
		m_BoxMax  = other.m_BoxMax;
		m_BoxMin  = other.m_BoxMin;
		m_Shape   = other.m_Shape;
		m_Radius  = other.m_Radius;
	}

	ParticleShapeGenerator& ParticleShapeGenerator::operator=(const ParticleShapeGenerator& other)
	{
		std::scoped_lock lock(other.m_Mutex, m_Mutex);
		m_BoxMax = other.m_BoxMax;
		m_BoxMin = other.m_BoxMin;
		m_Shape = other.m_Shape;
		m_Radius = other.m_Radius;
		return *this;
	}

	void ParticleShapeGenerator::Generate(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const
	{
		if (m_Shape == EmitShape::Box)
			generateBox(data, startId, endId);
		else if (m_Shape == EmitShape::Circle)
			generateCircle(data, startId, endId);
	}
	void ParticleShapeGenerator::SetEmitShape(EmitShape shape)
	{
		std::scoped_lock lock(m_Mutex);
		m_Shape = shape;
	}
	void ParticleShapeGenerator::SetBoxMin(const glm::vec3& boxMin)
	{
		std::scoped_lock lock(m_Mutex);
		m_BoxMin = boxMin;
	}

	void ParticleShapeGenerator::SetBoxMax(const glm::vec3& boxMax)
	{
		std::scoped_lock lock(m_Mutex);
		m_BoxMax = boxMax;
	}

	void ParticleShapeGenerator::SetRadius(float radius)
	{
		std::scoped_lock lock(m_Mutex);
		m_Radius = radius;
	}

	EmitShape ParticleShapeGenerator::GetEmitShape() const
	{
		std::scoped_lock lock(m_Mutex);
		return m_Shape;
	}

	glm::vec3 ParticleShapeGenerator::GetBoxMin() const
	{
		std::scoped_lock lock(m_Mutex);
		return m_BoxMin;
	}

	glm::vec3 ParticleShapeGenerator::GetBoxMax() const
	{
		std::scoped_lock lock(m_Mutex);
		return m_BoxMax;
	}

	float ParticleShapeGenerator::GetRadius() const
	{
		std::scoped_lock lock(m_Mutex);
		return m_Radius;
	}

	void ParticleShapeGenerator::generateBox(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const
	{
		std::scoped_lock lock(m_Mutex);

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
		std::scoped_lock lock(m_Mutex);

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
		m_LifeTime(5.0f)
	{
	}

	ParticleLifeGenerator::ParticleLifeGenerator(const ParticleLifeGenerator& other)
	{
		std::scoped_lock lock(other.m_Mutex);
		m_LifeTime = other.m_LifeTime;
	}
	ParticleLifeGenerator& ParticleLifeGenerator::operator=(const ParticleLifeGenerator& other)
	{
		std::scoped_lock lock(other.m_Mutex, m_Mutex);
		m_LifeTime = other.m_LifeTime;
		return *this;
	}
	void ParticleLifeGenerator::Generate(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const
	{
		std::scoped_lock lock(m_Mutex);
		for (uint32_t i = startId; i < endId; i++)
		{
			data->m_Particle[i].LifeRemaining = m_LifeTime;
		}
	}

	void ParticleLifeGenerator::SetLifeTime(float life)
	{
		std::scoped_lock lock(m_Mutex);
		m_LifeTime = life;
	}

	float ParticleLifeGenerator::GetLifeTime() const
	{
		std::scoped_lock lock(m_Mutex);
		return m_LifeTime;
	}

	ParticleRandomVelocityGenerator::ParticleRandomVelocityGenerator()
		:
		m_MinVelocity(-1.0f, -1.0f, 0.0f),
		m_MaxVelocity(1.0f, 1.0f, 0.0f)
	{
	}
	ParticleRandomVelocityGenerator::ParticleRandomVelocityGenerator(const ParticleRandomVelocityGenerator& other)
	{
		std::scoped_lock lock(other.m_Mutex);
		m_MaxVelocity = other.m_MaxVelocity;
		m_MinVelocity = other.m_MinVelocity;
	}

	ParticleRandomVelocityGenerator& ParticleRandomVelocityGenerator::operator=(const ParticleRandomVelocityGenerator& other)
	{
		std::scoped_lock lock(other.m_Mutex, m_Mutex);
		m_MaxVelocity = other.m_MaxVelocity;
		m_MinVelocity = other.m_MinVelocity;
		return *this;
	}
	void ParticleRandomVelocityGenerator::Generate(ParticleDataBuffer* data, uint32_t startId, uint32_t endId) const
	{
		std::scoped_lock lock(m_Mutex);
		for (uint32_t i = startId; i < endId; i++)
		{
			data->m_Particle[i].Velocity = glm::linearRand(m_MinVelocity, m_MaxVelocity);
		}
	}

	void ParticleRandomVelocityGenerator::SetMinVelocity(const glm::vec3& minVelocity)
	{
		std::scoped_lock lock(m_Mutex);
		m_MinVelocity = minVelocity;
	}

	void ParticleRandomVelocityGenerator::SetMaxVelocity(const glm::vec3& maxVelocity)
	{
		std::scoped_lock lock(m_Mutex);
		m_MaxVelocity = maxVelocity;
	}

	glm::vec3 ParticleRandomVelocityGenerator::GetMinVelocity() const
	{
		std::scoped_lock lock(m_Mutex);
		return m_MinVelocity;
	}



	glm::vec3 ParticleRandomVelocityGenerator::GetMaxVelocity() const
	{
		std::scoped_lock lock(m_Mutex);
		return m_MaxVelocity;
	}
}