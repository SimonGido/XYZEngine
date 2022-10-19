#include "stdafx.h"
#include "ParticleEmitterModuleGPU.h"

#include <glm/common.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>

namespace XYZ {
	BoxParticleEmitterModuleGPU::BoxParticleEmitterModuleGPU()
		:
		BoxMinimum(-1.0f),
		BoxMaximum(1.0f),
		Enabled(true)
	{
	}

	glm::vec3 BoxParticleEmitterModuleGPU::Generate() const
	{
		return glm::linearRand(BoxMinimum, BoxMaximum);
	}
}