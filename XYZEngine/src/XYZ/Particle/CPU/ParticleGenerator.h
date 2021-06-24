#pragma once
#include "ParticleData.h"

namespace XYZ {

	class ParticleGenerator
	{
	public:
		virtual ~ParticleGenerator() = default;
		virtual void Generate(ParticleDataBuffer* data, uint32_t startId, float timeStep) = 0;


		float EmittedParticles = 0.0f;
		float EmitRate = 50000.0f;
		float LifeTime = 2.0f;
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