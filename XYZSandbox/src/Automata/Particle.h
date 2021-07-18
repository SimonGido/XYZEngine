#pragma once

namespace Project {
	class Grid;
	struct Particle
	{
		void Update(uint32_t x, uint32_t y, Grid& grid);

		uint32_t m_VelocityX =  0;
		uint32_t m_VelocityY = -1;

		float m_InertialResistance;
		float m_Mass;
		float m_Density;
		float m_Temperature;
		float m_Friction;

		bool  m_Active = false;
		bool  m_IsWater = false;
	};
}