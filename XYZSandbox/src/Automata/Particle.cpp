#include "stdafx.h"
#include "Particle.h"

#include "Grid.h"

namespace Project {
	void Particle::Update(uint32_t x, uint32_t y, Grid& grid)
	{
		//if (y + m_VelocityY > 0 && y + m_VelocityY < grid.GetHeight())
		//{
		//	auto& targetParticle = grid.GetParticle(x, y + m_VelocityY);
		//	if (!targetParticle.m_Active)
		//	{
		//		grid.SwapParticles(x, y, x, y + m_VelocityY);
		//		return;
		//	}
		//}
		
		if (m_IsWater)
		{
			if (y > 0)
			{
				if (!grid.GetParticle(x, y - 1).m_Active)
				{
					grid.SwapParticles(x, y, x, y - 1);
					return;
				}
				if (x > 0)
				{
					// Particle left and down
					if (!grid.GetParticle(x - 1, y - 1).m_Active)
					{
						grid.SwapParticles(x, y, x - 1, y - 1);
						return;
					}
				}
				if (x < grid.GetWidth() - 1)
				{
					if (!grid.GetParticle(x + 1, y - 1).m_Active)
					{
						grid.SwapParticles(x, y, x + 1, y - 1);
						return;
					}
				}
			}
			if (x > 0)
			{
				// Particle left and down
				if (!grid.GetParticle(x - 1, y).m_Active)
				{
					grid.SwapParticles(x, y, x - 1, y);
					return;
				}
			}
			if (x < grid.GetWidth() - 1)
			{
				if (!grid.GetParticle(x + 1, y).m_Active)
				{
					grid.SwapParticles(x, y, x + 1, y);
					return;
				}
			}
			return;
		}

		if (y > 0)
		{
			if (!grid.GetParticle(x, y - 1).m_Active)
			{
				grid.SwapParticles(x, y, x, y - 1);
				return;
			}
			if (x > 0)
			{
				if (!grid.GetParticle(x - 1, y - 1).m_Active)
				{
					grid.SwapParticles(x, y, x - 1, y - 1);
					return;
				}
			}
			if (x < grid.GetWidth() - 1)
			{
				if (!grid.GetParticle(x + 1, y - 1).m_Active)
				{
					grid.SwapParticles(x, y, x + 1, y - 1);
					return;
				}
			}
		}
	}
}