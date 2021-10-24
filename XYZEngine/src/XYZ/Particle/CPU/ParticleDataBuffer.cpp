#include "stdafx.h"
#include "ParticleDataBuffer.h"


namespace XYZ {
	ParticleDataBuffer::ParticleDataBuffer(uint32_t maxParticles)
		:
		m_MaxParticles(maxParticles),
		m_AliveParticles(0)
	{
		if (maxParticles)
		{
			generateParticles(maxParticles);
		}
		else
		{
			m_Particle		  = nullptr;
			m_TexCoord		  = nullptr;
			m_StartColor	  = nullptr;
			m_EndColor		  = nullptr;
			m_Size			  = nullptr;
			m_Lights		  = nullptr;
			m_AngularVelocity = nullptr;
			m_Rotation        = nullptr;
		}
	}
	ParticleDataBuffer::ParticleDataBuffer(ParticleDataBuffer&& other) noexcept
	{
		m_Particle = other.m_Particle;
		m_TexCoord = other.m_TexCoord;
		m_StartColor = other.m_StartColor;
		m_EndColor = other.m_EndColor;
		m_Size = other.m_Size;
		m_Lights = other.m_Lights;
		m_AngularVelocity = other.m_AngularVelocity;
		m_Rotation = other.m_Rotation;
		m_MaxParticles = other.m_MaxParticles;
		m_AliveParticles = other.m_AliveParticles;

		other.m_Particle = nullptr;
		other.m_TexCoord = nullptr;
		other.m_StartColor = nullptr;
		other.m_EndColor = nullptr;
		other.m_Size = nullptr;
		other.m_Lights = nullptr;
		other.m_AngularVelocity = nullptr;
		other.m_Rotation = nullptr;
		other.m_MaxParticles = 0;
		other.m_AliveParticles = 0;
	}
	ParticleDataBuffer::~ParticleDataBuffer()
	{
		deleteParticles();
	}
	ParticleDataBuffer& ParticleDataBuffer::operator=(ParticleDataBuffer&& other) noexcept
	{
		m_Particle = other.m_Particle;
		m_TexCoord = other.m_TexCoord;
		m_StartColor = other.m_StartColor;
		m_EndColor = other.m_EndColor;
		m_Size = other.m_Size;
		m_Lights = other.m_Lights;
		m_AngularVelocity = other.m_AngularVelocity;
		m_Rotation = other.m_Rotation;
		m_MaxParticles = other.m_MaxParticles;
		m_AliveParticles = other.m_AliveParticles;

		other.m_Particle = nullptr;
		other.m_TexCoord = nullptr;
		other.m_StartColor = nullptr;
		other.m_EndColor = nullptr;
		other.m_Size = nullptr;
		other.m_Lights = nullptr;
		other.m_AngularVelocity = nullptr;
		other.m_Rotation = nullptr;
		other.m_MaxParticles = 0;
		other.m_AliveParticles = 0;
		return *this;
	}
	void ParticleDataBuffer::SetMaxParticles(uint32_t maxParticles)
	{
		deleteParticles();
		m_MaxParticles = maxParticles;
		generateParticles(maxParticles);
		m_AliveParticles = 0;
	}
	void ParticleDataBuffer::Wake(uint32_t id)
	{
		m_Particle[id].Alive = true;
		m_AliveParticles++;
	}
	void ParticleDataBuffer::Kill(uint32_t id)
	{
		swapData(id, m_AliveParticles - 1);
		m_Particle[m_AliveParticles - 1].Alive = false;
		m_AliveParticles--;
	}
	void ParticleDataBuffer::generateParticles(uint32_t particleCount)
	{
		m_Particle		  = new Particle[particleCount];
		m_TexCoord		  = new glm::vec4[particleCount];
		m_StartColor      = new glm::vec4[particleCount];
		m_EndColor        = new glm::vec4[particleCount];
		m_Size			  = new glm::vec2[particleCount];
		m_Lights		  = new glm::vec3[particleCount];
		m_AngularVelocity = new float[particleCount];
		m_Rotation		  = new float[particleCount];

		for (size_t i = 0; i < particleCount; ++i)
			m_Particle[i].Alive = false;
	}
	void ParticleDataBuffer::swapData(uint32_t a, uint32_t b)
	{
		m_Particle[a]		 = m_Particle[b];
		m_TexCoord[a]		 = m_TexCoord[b];
		m_StartColor[a]		 = m_StartColor[b];
		m_EndColor[a]		 = m_EndColor[b];
		m_Size[a]			 = m_Size[b];
		m_Lights[a]			 = m_Lights[b];
		m_Rotation[a]		 = m_Rotation[b];
		m_AngularVelocity[a] = m_AngularVelocity[b];
	}

	void ParticleDataBuffer::deleteParticles()
	{
		delete[]m_Particle;
		delete[]m_TexCoord;
		delete[]m_StartColor;
		delete[]m_EndColor;
		delete[]m_Size;
		delete[]m_Lights;
		delete[]m_Rotation;
		delete[]m_AngularVelocity;
	}
}