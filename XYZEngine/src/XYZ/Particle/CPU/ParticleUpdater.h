#pragma once
#include "XYZ/Utils/DataStructures/ThreadPass.h"
#include "ParticleData.h"

#include <glm/glm.hpp>

namespace XYZ {

	class ParticleUpdater
	{
	public:
		virtual ~ParticleUpdater() = default;
		virtual void UpdateParticles(float timeStep, ParticleDataBuffer* data) = 0;
		virtual void Update() {};
	}; 


	class BasicTimerUpdater : public ParticleUpdater
	{
	public:
		virtual void UpdateParticles(float timeStep, ParticleDataBuffer* data) override;
	};

	class PositionUpdater : public ParticleUpdater
	{
	public:
		virtual void UpdateParticles(float timeStep, ParticleDataBuffer* data) override;
	};

	struct PointLight2D;
	class LightUpdater : public ParticleUpdater
	{
	public:
		LightUpdater();

		virtual void UpdateParticles(float timeStep, ParticleDataBuffer* data) override;
		virtual void Update() override;

		void SetMaxLights(uint32_t maxLights);	
		
		struct LigthtPassData
		{
			std::vector <glm::vec3> LightPositions;
			uint32_t				LightCount = 0;
		};

		ThreadPass<LigthtPassData>  LightBuffer;
		PointLight2D*			    Light;
	private:
		uint32_t					MaxLights;
	};
}