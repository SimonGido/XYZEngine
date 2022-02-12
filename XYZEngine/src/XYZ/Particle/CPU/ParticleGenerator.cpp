#include "stdafx.h"
#include "ParticleGenerator.h"


#include <glm/common.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>

namespace XYZ {

	ParticleShapeGenerator::ParticleShapeGenerator()
		:
		Shape(EmitShape::Box),
		BoxMin(-1.0f, -1.0, 0.0f),
		BoxMax(1.0f, 1.0f, 0.0f),
		Radius(7.0f),
		Enabled(true)
	{
	}

	ParticleShapeGenerator::ParticleShapeGenerator(const ParticleShapeGenerator& other)
	{
		BoxMax  = other.BoxMax;
		BoxMin  = other.BoxMin;
		Shape   = other.Shape;
		Radius  = other.Radius;
		Enabled = other.Enabled;
	}

	ParticleShapeGenerator& ParticleShapeGenerator::operator=(const ParticleShapeGenerator& other)
	{
		BoxMax  = other.BoxMax;
		BoxMin  = other.BoxMin;
		Shape   = other.Shape;
		Radius  = other.Radius;
		Enabled = other.Enabled;
		return *this;
	}

	void ParticleShapeGenerator::Generate(ParticleDataBuffer& data, uint32_t startId, uint32_t endId) const
	{
		if (Enabled)
		{
			if (Shape == EmitShape::Box)
				generateBox(data, startId, endId);
			else if (Shape == EmitShape::Circle)
				generateCircle(data, startId, endId);
		}
	}
	

	void ParticleShapeGenerator::generateBox(ParticleDataBuffer& data, uint32_t startId, uint32_t endId) const
	{
		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_real_distribution<double> dist(-1.0, 1.0); // distribution in range [1, 6]

		endId = std::min(endId, data.GetMaxParticles());
		for (uint32_t i = startId; i < endId; i++)
		{
			data.Particle[i].Color	 = glm::linearRand(glm::vec4(0.0f), glm::vec4(1.0f));
			data.Particle[i].Position  = glm::linearRand(BoxMin, BoxMax);
			data.TexOffset[i]			 = glm::vec2(0.0f, 0.0f);
			data.Size[i]				 = glm::vec3(0.5f);
			data.Rotation[i]			 = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		}
	}

	void ParticleShapeGenerator::generateCircle(ParticleDataBuffer& data, uint32_t startId, uint32_t endId) const
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
			data.Particle[i].Position  = glm::vec3(point.x, point.y, 0.0f);
			data.Particle[i].Color	 = glm::vec4(glm::linearRand(0.0f, 1.0f));
			data.TexOffset[i]			 = glm::vec2(0.0f, 0.0f);
			data.Size[i]				 = glm::vec3(0.5f);
			data.Rotation[i]			 = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		}
	}


	ParticleLifeGenerator::ParticleLifeGenerator()
		:
		LifeTime(5.0f),
		Enabled(true)
	{
	}

	ParticleLifeGenerator::ParticleLifeGenerator(const ParticleLifeGenerator& other)
	{
		LifeTime = other.LifeTime;
		Enabled  = other.Enabled;
	}
	ParticleLifeGenerator& ParticleLifeGenerator::operator=(const ParticleLifeGenerator& other)
	{
		LifeTime = other.LifeTime;
		Enabled  = other.Enabled;
		return *this;
	}
	void ParticleLifeGenerator::Generate(ParticleDataBuffer& data, uint32_t startId, uint32_t endId) const
	{
		if (Enabled)
		{
			for (uint32_t i = startId; i < endId; i++)
			{
				data.Particle[i].LifeRemaining = LifeTime;
			}
		}
	}

	ParticleRandomVelocityGenerator::ParticleRandomVelocityGenerator()
		:
		MinVelocity(-1.0f, -1.0f, 0.0f),
		MaxVelocity(1.0f, 1.0f, 0.0f),
		Enabled(true)
	{
	}
	ParticleRandomVelocityGenerator::ParticleRandomVelocityGenerator(const ParticleRandomVelocityGenerator& other)
	{
		MaxVelocity = other.MaxVelocity;
		MinVelocity = other.MinVelocity;
		Enabled = other.Enabled;
	}

	ParticleRandomVelocityGenerator& ParticleRandomVelocityGenerator::operator=(const ParticleRandomVelocityGenerator& other)
	{
		MaxVelocity = other.MaxVelocity;
		MinVelocity = other.MinVelocity;
		Enabled = other.Enabled;
		return *this;
	}
	void ParticleRandomVelocityGenerator::Generate(ParticleDataBuffer& data, uint32_t startId, uint32_t endId) const
	{
		if (Enabled)
		{
			for (uint32_t i = startId; i < endId; i++)
			{
				data.Particle[i].Velocity = glm::linearRand(MinVelocity, MaxVelocity);
			}
		}
	}
}