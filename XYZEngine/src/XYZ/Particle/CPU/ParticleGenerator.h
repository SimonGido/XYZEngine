#pragma once
#include "ParticleData.h"

namespace XYZ {

	class ParticleGenerator
	{
	public:
		ParticleGenerator();

		virtual ~ParticleGenerator() = default;
		virtual void Generate(ParticleDataBuffer* data, uint32_t startId, float timeStep) = 0;


		float EmittedParticles;
		float EmitRate;
		float LifeTime;
	};

	class ParticleBoxGenerator : public ParticleGenerator
	{
	public:
		ParticleBoxGenerator();

		virtual void Generate(ParticleDataBuffer* data, uint32_t startId, float timeStep) override;
		
		glm::vec4 BoxMin;
		glm::vec4 BoxMax;
	};

}