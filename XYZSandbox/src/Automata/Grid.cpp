#include "stdafx.h"
#include "Grid.h"


namespace Project {
	Grid::Grid(uint32_t width, uint32_t height)
		:
		m_Width(width),
		m_Height(height)
	{
		m_Particles.resize((size_t)width * (size_t)height);
		for (auto& particle : m_Particles)
			particle.m_Active = false;
	}
	void Grid::SwapParticles(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1)
	{
		std::swap(m_Particles[m_Width * y0 + x0], m_Particles[m_Width * y1 + x1]);
	}
	Particle& Grid::GetParticle(uint32_t x, uint32_t y)
	{
		return m_Particles[m_Width * y + x];
	}
}