#pragma once
#include <glm/glm.hpp>

namespace XYZ {


	class BoxParticleEmitterModuleGPU
	{
	public:
		BoxParticleEmitterModuleGPU();

		glm::vec3 Generate() const;

		glm::vec3 BoxMinimum;
		glm::vec3 BoxMaximum;
		bool	  Enabled;
	};
}