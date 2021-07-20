#pragma once
#include "Particle.h"

namespace Project {
	class Grid
	{
	public:
		Grid(uint32_t width, uint32_t height);

		void      SwapParticles(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1);
		Particle& GetParticle(uint32_t x, uint32_t y);
		uint32_t  GetWidth() const { return m_Width; }
		uint32_t  GetHeight() const { return m_Height; }


		const std::vector<Particle>& GetParticles() const { return m_Particles; }
	private:
		std::vector<Particle> m_Particles;
		std::vector<Particle> m_NextGeneration;

		uint32_t			  m_Width;
		uint32_t			  m_Height;
	};
}