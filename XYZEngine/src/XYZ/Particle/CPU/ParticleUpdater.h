#pragma once
#include "XYZ/Utils/DataStructures/ThreadPass.h"
#include "XYZ/Physics/PhysicsWorld2D.h"
#include "ParticleDataBuffer.h"


#include <glm/glm.hpp>

namespace XYZ {

	class LightUpdater
	{
	public:
		LightUpdater();

		void UpdateParticles(float ts, ParticleDataBuffer& data);
		void Reset();

		struct Light
		{
			glm::vec3 Color     = glm::vec3(1.0f);
			float	  Radius    = 1.0f;
			float	  Intensity = 1.0f;
		} Light;
		
		std::vector<glm::vec3> Lights;
		uint32_t			   MaxLights;	
		bool				   Enabled = true;
	};

	class TextureAnimationUpdater
	{
	public:
		TextureAnimationUpdater();

		void UpdateParticles(float ts, ParticleDataBuffer& data) const;

		glm::ivec2 Tiles;
		uint32_t   StartFrame;
		float      CycleLength;
		bool	   Enabled = true;
	};

	class RotationOverLife
	{
	public:
		RotationOverLife();

		void UpdateParticles(float ts, ParticleDataBuffer& data) const;


		glm::vec3 EulerAngles;
		float	  CycleLength;
		bool	  Enabled = true;
	};

}