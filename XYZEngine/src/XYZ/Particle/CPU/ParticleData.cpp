#include "stdafx.h"
#include "ParticleData.h"


namespace XYZ {
	ParticleDataBuffer::ParticleDataBuffer(uint32_t maxParticles)
		:
		m_MaxParticles(maxParticles),
		m_AliveParticles(0)
	{
		generateParticles(maxParticles);
	}
	ParticleDataBuffer::~ParticleDataBuffer()
	{
		delete[]m_Position;
		delete[]m_Color;
		delete[]m_StartColor;
		delete[]m_EndColor;
		delete[]m_Velocity;
		delete[]m_Rotation;
		delete[]m_LifeRemaining;
		delete[]m_Alive;		
		delete[]m_StartColor;
		delete[]m_EndColor;		
		delete[]m_AngularVelocity;
	}
	void ParticleDataBuffer::Wake(uint32_t id)
	{
		m_Alive[id] = true;
		m_AliveParticles++;
	}
	void ParticleDataBuffer::Kill(uint32_t id)
	{
		swapData(id, m_AliveParticles - 1);
		m_Alive[m_AliveParticles - 1] = false;
		m_AliveParticles--;
	}
	void ParticleDataBuffer::generateParticles(uint32_t particleCount)
	{
		m_StartColor      = new glm::vec4[particleCount];
		m_EndColor        = new glm::vec4[particleCount];
		m_AngularVelocity = new float[particleCount];
		m_Position		  = new glm::vec3[particleCount];
		m_Color			  = new glm::vec4[particleCount];
		m_Velocity		  = new glm::vec3[particleCount];
		m_Rotation		  = new float[particleCount];
		m_LifeRemaining   = new float[particleCount];
		m_Alive			  = new bool[particleCount];
	}
	void ParticleDataBuffer::swapData(uint32_t a, uint32_t b)
	{
		m_Position[a]		 = m_Position[b];
		m_Color[a]			 = m_Color[b];
		m_Velocity[a]		 = m_Velocity[b];
		m_Rotation[a]		 = m_Rotation[b];
		m_LifeRemaining[a]	 = m_LifeRemaining[b];
		m_Alive[a]			 = m_Alive[b];
		m_StartColor[a]		 = m_StartColor[b];
		m_EndColor[a]		 = m_EndColor[b];	
		m_AngularVelocity[a] = m_AngularVelocity[b];
	}
}