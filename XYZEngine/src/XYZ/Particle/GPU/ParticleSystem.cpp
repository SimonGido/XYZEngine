#include "stdafx.h"
#include "ParticleSystem.h"
#include "XYZ/Renderer/Renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <random>

namespace XYZ {
	
	static std::vector<glm::vec2> RandomPointsInCircle(const glm::vec2& center, float radius, uint32_t count)
	{
		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);

		std::vector<glm::vec2> points;
		points.reserve(count);
		for (uint32_t i = 0; i < count; ++i)
		{
			float r = radius * (float)sqrt(dist(rng));
			float theta = dist(rng) * 2.0f * glm::pi<float>();
			glm::vec2 point(
				center.x + radius * cos(theta),
				center.y + radius * sin(theta)
			);
			points.push_back(point);
		}
		return points;
	}

	static std::vector<glm::vec2> RandomPointsInRectangle(const glm::vec2& min, const glm::vec2& max, uint32_t count)
	{
		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_real_distribution<double> distX(min.x, max.x);
		std::uniform_real_distribution<double> distY(min.y, max.y);
		
		std::vector<glm::vec2> points;
		points.reserve(count);
		for (uint32_t i = 0; i < count; ++i)
		{
			points.push_back(glm::vec2(distX(rng), distY(rng)));
		}
		return points;
	}

	ParticleEmitter::ParticleEmitter()
		:
		Shape(ParticleEmissionShape::Sphere),
		RectangleMin(0.0f),
		RectangleMax(1.0f),
		Center(0.0f),
		Radius(1.0f)
	{
	}

	ParticleSystem::ParticleSystem(const Ref<ParticleMaterial>& material)
		:
		m_EmittedParticles(0.0f),
		m_PlayTime(0.0f),
		m_Rate(1.0f),
		m_Playing(true)
	{
		m_Renderer.ParticleMaterial = material;
	}
	void ParticleSystem::Update(Timestep ts)
	{
		if (m_Playing)
		{		
			float raise = m_Rate * ts;
			
			if (m_EmittedParticles + raise <= m_Renderer.ParticleMaterial->GetMaxParticles())
				m_EmittedParticles += raise;
			else
				m_EmittedParticles = (float)m_Renderer.ParticleMaterial->GetMaxParticles();
				
			uint32_t emitted = (uint32_t)std::ceil(m_EmittedParticles);
			m_Renderer.ParticleMaterial->SetParticleBuffersElementCount(emitted);
			m_PlayTime += ts;
		}
	}
	void ParticleSystem::Reset()
	{
		m_Renderer.ParticleMaterial->ResetCounters();
		m_PlayTime = 0.0f;
		m_EmittedParticles = 0.0f;
	}
	void ParticleSystem::GenerateParticleData()
	{
		std::vector<glm::vec2> points;
		if (m_Emitter.Shape == ParticleEmissionShape::Sphere)
		{
			points = std::move(RandomPointsInCircle(m_Emitter.Center, m_Emitter.Radius, m_Renderer.ParticleMaterial->GetMaxParticles()));
		}
		else if (m_Emitter.Shape == ParticleEmissionShape::Rectangle)
		{
			points = std::move(RandomPointsInRectangle(m_Emitter.RectangleMin, m_Emitter.RectangleMax, m_Renderer.ParticleMaterial->GetMaxParticles()));
		}
	}

	void ParticleSystem::SetParticleEmitter(const ParticleEmitter& emitter)
	{
		m_Emitter = emitter;
		GenerateParticleData();
	}
	
	void ParticleRendererGPU::Bind()
	{
		ParticleMaterial->GetVertexArray()->Bind();
		ParticleMaterial->GetIndirectBuffer()->Bind();
		Renderer::DrawElementsIndirect(nullptr);
	}

}