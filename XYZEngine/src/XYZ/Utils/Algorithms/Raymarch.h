#pragma once
#include "XYZ/Utils/Math/Ray.h"

namespace XYZ {

	class XYZ_API Raymarch
	{
	public:
		Raymarch(const Ray& ray, uint32_t width, uint32_t height, uint32_t depth, float cellSize);

		void Step();

		bool  IntersectsGrid() const { return m_IntersectsGrid; }
		float GridIntersectionT() const { return m_GridIntersectionT; }
		
		const glm::ivec3&	GetStartVoxel() const { return m_StartVoxel; }
		const glm::ivec3&	GetCurrentVoxel() const { return m_CurrentVoxel; };
		const glm::vec3&	GetHitNormal() const { return m_HitNormal; }
		float				GetDistanceTraveled() const { return m_DistanceTraveled; }
	private:
		Ray		   m_Ray;
		glm::ivec3 m_StartVoxel;
		glm::ivec3 m_CurrentVoxel;
		glm::ivec3 m_Step;
		glm::vec3  m_Delta;
		glm::vec3  m_Max;
		glm::vec3  m_HitNormal;

		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_Depth;
		float	 m_Size;
		float	 m_DistanceTraveled;

		bool	 m_IntersectsGrid;
		float	 m_GridIntersectionT;
	};
}