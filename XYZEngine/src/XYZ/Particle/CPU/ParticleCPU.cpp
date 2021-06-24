#include "stdafx.h"
#include "ParticleCPU.h"

#include "ParticleModule.h"

namespace XYZ {
	void ParticleCPU::Process(ParticleThreadPass& pass, float timeStep)
	{
		if (LifeRemaining <= 0.0f)
		{
			// Copy last alive at the place of dead particle
			*this = pass.ParticlePool[pass.ParticlesAlive - 1];
			pass.ParticlePool[pass.ParticlesAlive - 1].Alive = false;
			pass.ParticlesAlive--;
		}
		else
		{
			LifeRemaining -= timeStep;
			Position += Velocity * timeStep;
			Rotation += AngularVelocity * timeStep;
		}
	}
}