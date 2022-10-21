#include "stdafx.h"
#include "ParticleSystemGPU.h"

#include <glm/common.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>

namespace XYZ {
	/*
	ParticleSystemGPU::ParticleSystemGPU()
	{		
		glm::vec4 MinVelocity(-0.5f);
		glm::vec4 MaxVelocity(0.5f);
		for (size_t i = 0; i < ParticleProperties.size(); ++i)
		{
			float startScale = glm::linearRand(0.1f, 2.0f);
			float endScale = glm::linearRand(0.1f, 2.0f);

			ParticleProperties[i].StartPosition = glm::vec4(0.0f);
			ParticleProperties[i].StartColor	= glm::linearRand(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), glm::vec4(1.0f));
			ParticleProperties[i].StartRotation = glm::linearRand(glm::vec4(-90.0f), glm::vec4(90.0f));
			ParticleProperties[i].StartScale	= glm::vec4(startScale);
			ParticleProperties[i].StartVelocity = glm::linearRand(MinVelocity, MaxVelocity);
		
			ParticleProperties[i].EndColor		= glm::linearRand(glm::vec4(0.8f, 0.4f, 0.5f, 1.0f), glm::vec4(1.0f));
			ParticleProperties[i].EndRotation	= glm::linearRand(glm::vec4(-90.0f), glm::vec4(90.0f));
			ParticleProperties[i].EndScale		= glm::vec4(endScale);
			ParticleProperties[i].EndVelocity	= glm::linearRand(MinVelocity, MaxVelocity);

			ParticleProperties[i].LifeTime = glm::linearRand(1.0f, 5.0f);
		}
	}
	*/
	ParticleSystemGPU::ParticleSystemGPU(ParticleSystemLayout layout)
		:
		m_Layout(layout)
	{
	}
	uint32_t ParticleSystemGPU::Update(Timestep ts, std::byte* particleBuffer, uint32_t bufferSize)
	{
		uint32_t emissionCount = 0;
		for (auto& emitter : ParticleEmitters)
		{
			emissionCount += emitter.Emit(ts, particleBuffer, bufferSize);
		}

		return emissionCount;
	}
	
	ParticleBuffer::ParticleBuffer(uint32_t maxParticles, uint32_t stride)
		:
		m_Stride(stride)
	{
		m_Data.resize(maxParticles * stride);
	}

	void ParticleBuffer::SetMaxParticles(uint32_t maxParticles, uint32_t stride)
	{
		m_Stride = stride;
		m_Data.resize(maxParticles * stride);
		memset(m_Data.data(), 0, m_Data.size());
	}

	std::byte* ParticleBuffer::GetData(uint32_t offsetParticles)
	{
		uint32_t offset = offsetParticles * m_Stride;
		return &m_Data.data()[offset];
	}

}