#pragma once
#include "XYZ/Core/Ref/Ref.h"
#include "ParticleDataBuffer.h"

#include <mutex>

namespace XYZ {


	class ParticleGenerator
	{
	public:
		void Generate(ParticleDataBuffer& data, uint32_t startId, uint32_t endId) const;

		glm::vec4 Color = glm::vec4(1.0f);
		glm::vec3 Size = glm::vec3(1.0f);
		bool Enabled = true;
	};

	enum class EmitShape
	{
		Box,
		Circle
	};

	class ParticleShapeGenerator
	{
	public:
		ParticleShapeGenerator();
		ParticleShapeGenerator(const ParticleShapeGenerator& other);
		ParticleShapeGenerator& operator=(const ParticleShapeGenerator& other);

		void Generate(ParticleDataBuffer& data, uint32_t startId, uint32_t endId) const;


		EmitShape	Shape;
		glm::vec3	BoxMin;
		glm::vec3	BoxMax;
		float		Radius;
		bool	    Enabled;

	private:
		void generateBox(ParticleDataBuffer& data, uint32_t startId, uint32_t endId) const;
		void generateCircle(ParticleDataBuffer& data, uint32_t startId, uint32_t endId) const;

	};

	class ParticleLifeGenerator
	{
	public:
		ParticleLifeGenerator();
		ParticleLifeGenerator(const ParticleLifeGenerator& other);
		ParticleLifeGenerator& operator =(const ParticleLifeGenerator& other);

		void  Generate(ParticleDataBuffer& data, uint32_t startId, uint32_t endId) const;

		float LifeTime;
		bool  Enabled;
	};


	class ParticleRandomVelocityGenerator
	{
	public:
		ParticleRandomVelocityGenerator();
		ParticleRandomVelocityGenerator(const ParticleRandomVelocityGenerator& other);
		ParticleRandomVelocityGenerator& operator=(const ParticleRandomVelocityGenerator& other);

		void Generate(ParticleDataBuffer& data, uint32_t startId, uint32_t endId) const;

		glm::vec3 MinVelocity;
		glm::vec3 MaxVelocity;
		bool	  Enabled;
	};
}