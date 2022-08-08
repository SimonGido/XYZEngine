#include "stdafx.h"
#include "ParticlePool.h"


namespace XYZ {


	ParticlePool::ParticlePool(const uint32_t maxParticles)
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
			Particles	= nullptr;
		}
	}
	ParticlePool::ParticlePool(ParticlePool&& other) noexcept
	{
		Particles = other.Particles;
	
		m_MaxParticles = other.m_MaxParticles;
		m_AliveParticles = other.m_AliveParticles;

		other.Particles = nullptr;
		other.m_MaxParticles = 0;
		other.m_AliveParticles = 0;
	}
	ParticlePool::~ParticlePool()
	{
		deleteParticles();
	}

	ParticlePool::ParticlePool(const ParticlePool& other)
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
			Particles = nullptr;
		}
	}
	
	ParticlePool& ParticlePool::operator=(const ParticlePool& other)
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
			Particles = nullptr;
		}	
		return *this;
	}

	ParticlePool& ParticlePool::operator=(ParticlePool&& other) noexcept
	{
		deleteParticles();
		Particles = other.Particles;

		m_MaxParticles = other.m_MaxParticles;
		m_AliveParticles = other.m_AliveParticles;

		other.Particles = nullptr;
		other.m_MaxParticles = 0;
		other.m_AliveParticles = 0;
		return *this;
	}
	void ParticlePool::SetMaxParticles(uint32_t maxParticles)
	{
		deleteParticles();
		m_MaxParticles = maxParticles;
		generateParticles(maxParticles);
		m_AliveParticles = 0;
	}

	void ParticlePool::Wake(uint32_t id)
	{
		XYZ_ASSERT(m_AliveParticles < m_MaxParticles, "");
		Particles[id].Alive = true;
		m_AliveParticles++;
	}
	void ParticlePool::Kill(uint32_t id)
	{
		swapData(id, m_AliveParticles - 1);
		Particles[m_AliveParticles - 1].Alive = false;
		m_AliveParticles--;
	}
	void ParticlePool::generateParticles(uint32_t particleCount)
	{
		Particles = new Particle[particleCount];
		for (size_t i = 0; i < particleCount; ++i)
			Particles[i].Alive = false;
	}
	void ParticlePool::swapData(uint32_t a, uint32_t b)
	{
		Particles[a]	= Particles[b];
	}

	template <typename T>
	static void copy(T* dest, T* source, uint32_t count)
	{
		memcpy(dest, source, count * sizeof(T));
	}

	void ParticlePool::copyData(const ParticlePool& source)
	{
		copy(Particles,   source.Particles,	m_MaxParticles);	
	}

	void ParticlePool::deleteParticles()
	{
		delete[]Particles;
	}
}