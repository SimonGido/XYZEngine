#pragma once
#include "ParticleData.h"


namespace XYZ {

	class ParticleUpdater
	{
	public:
		virtual ~ParticleUpdater() = default;
		virtual void Update(float timeStep, ParticleDataBuffer* data) = 0;
	};


	class BasicTimerUpdater : public ParticleUpdater
	{
	public:
		virtual void Update(float timeStep, ParticleDataBuffer* data) override;
	};
}