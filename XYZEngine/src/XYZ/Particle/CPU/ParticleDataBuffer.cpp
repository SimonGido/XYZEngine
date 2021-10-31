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
			m_TexOffset		  = nullptr;
			m_StartColor	  = nullptr;
			m_EndColor		  = nullptr;
			m_Size			  = nullptr;
			m_Rotation        = nullptr;
		}
	}
	ParticleDataBuffer::ParticleDataBuffer(ParticleDataBuffer&& other) noexcept
	{
		m_Particle = other.m_Particle;
		m_TexOffset = other.m_TexOffset;
		m_StartColor = other.m_StartColor;
		m_EndColor = other.m_EndColor;
		m_Size = other.m_Size;
		m_Rotation = other.m_Rotation;

		m_MaxParticles = other.m_MaxParticles;
		m_AliveParticles = other.m_AliveParticles;

		other.m_Particle = nullptr;
		other.m_TexOffset = nullptr;
		other.m_StartColor = nullptr;
		other.m_EndColor = nullptr;
		other.m_Size = nullptr;
		other.m_Rotation = nullptr;
		other.m_MaxParticles = 0;
		other.m_AliveParticles = 0;
	}
	ParticleDataBuffer::~ParticleDataBuffer()
	{
		deleteParticles();
	}

	ParticleDataBuffer::ParticleDataBuffer(const ParticleDataBuffer& other)
		:
		m_MaxParticles(other.m_MaxParticles),
		m_AliveParticles(other.m_AliveParticles)
	{
		if (m_MaxParticles)
		{
			generateParticles(m_MaxParticles);
			copyData(other);
		}
		else
		{
			m_Particle = nullptr;
			m_TexOffset = nullptr;
			m_StartColor = nullptr;
			m_EndColor = nullptr;
			m_Size = nullptr;
			m_Rotation = nullptr;
		}
	}
	
	ParticleDataBuffer& ParticleDataBuffer::operator=(const ParticleDataBuffer& other)
	{
		m_MaxParticles = other.m_MaxParticles;
		m_AliveParticles = other.m_AliveParticles;
		
		deleteParticles();
		if (m_MaxParticles)
		{
			generateParticles(m_MaxParticles);
			copyData(other);
		}
		else
		{
			m_Particle = nullptr;
			m_TexOffset = nullptr;
			m_StartColor = nullptr;
			m_EndColor = nullptr;
			m_Size = nullptr;
			m_Rotation = nullptr;
		}	
		return *this;
	}

	ParticleDataBuffer& ParticleDataBuffer::operator=(ParticleDataBuffer&& other) noexcept
	{
		deleteParticles();
		m_Particle = other.m_Particle;
		m_TexOffset = other.m_TexOffset;
		m_StartColor = other.m_StartColor;
		m_EndColor = other.m_EndColor;
		m_Size = other.m_Size;
		m_Rotation = other.m_Rotation;

		m_MaxParticles = other.m_MaxParticles;
		m_AliveParticles = other.m_AliveParticles;

		other.m_Particle = nullptr;
		other.m_TexOffset = nullptr;
		other.m_StartColor = nullptr;
		other.m_EndColor = nullptr;
		other.m_Size = nullptr;
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
		m_TexOffset		  = new glm::vec2[particleCount];
		m_StartColor      = new glm::vec4[particleCount];
		m_EndColor        = new glm::vec4[particleCount];
		m_Size			  = new glm::vec3[particleCount];
		m_Rotation		  = new glm::quat[particleCount];
		for (size_t i = 0; i < particleCount; ++i)
			m_Particle[i].Alive = false;
	}
	void ParticleDataBuffer::swapData(uint32_t a, uint32_t b)
	{
		m_Particle[a]		 = m_Particle[b];
		m_TexOffset[a]		 = m_TexOffset[b];
		m_StartColor[a]		 = m_StartColor[b];
		m_EndColor[a]		 = m_EndColor[b];
		m_Size[a]			 = m_Size[b];
		m_Rotation[a]		 = m_Rotation[b];
	}

	template <typename T>
	static void copy(T* dest, T* source, uint32_t count)
	{
		memcpy(dest, source, count * sizeof(T));
	}

	void ParticleDataBuffer::copyData(const ParticleDataBuffer& source)
	{
		copy(m_Particle,   source.m_Particle,	m_MaxParticles);
		copy(m_TexOffset,  source.m_TexOffset,	m_MaxParticles);
		copy(m_StartColor, source.m_StartColor,	m_MaxParticles);
		copy(m_EndColor,   source.m_EndColor,	m_MaxParticles);
		copy(m_Size,	   source.m_Size,		m_MaxParticles);
		copy(m_Rotation,   source.m_Rotation,	m_MaxParticles);	
	}

	void ParticleDataBuffer::deleteParticles()
	{
		delete[]m_Particle;
		delete[]m_TexOffset;
		delete[]m_StartColor;
		delete[]m_EndColor;
		delete[]m_Size;
		delete[]m_Rotation;
	}
}