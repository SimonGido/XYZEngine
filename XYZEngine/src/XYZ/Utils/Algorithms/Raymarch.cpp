#include "stdafx.h"
#include "Raymarch.h"

namespace XYZ {
	Raymarch::Raymarch(const Ray& ray, uint32_t width, uint32_t height, uint32_t depth, float cellSize)
		:
		m_Width(width),
		m_Height(height),
		m_Depth(depth),
		m_Size(cellSize),
        m_GridIntersectionT(0.0f),
		m_DistanceTraveled(0.0f),
		m_Ray(ray),
		m_HitNormal(0.0f)
	{
        AABB aabb(glm::vec3(0.0f), glm::vec3(width, height, depth));
        m_IntersectsGrid = ray.IntersectsAABB(aabb, m_GridIntersectionT);
       
        m_CurrentVoxel = glm::ivec3(floor(ray.Origin / cellSize));
		m_StartVoxel = m_CurrentVoxel;

        m_Step = glm::ivec3(
            (ray.Direction.x > 0.0) ? 1 : -1,
            (ray.Direction.y > 0.0) ? 1 : -1,
            (ray.Direction.z > 0.0) ? 1 : -1
        );
        glm::vec3 next_boundary = glm::vec3(
            float((m_Step.x > 0) ? m_CurrentVoxel.x + 1 : m_CurrentVoxel.x) * cellSize,
            float((m_Step.y > 0) ? m_CurrentVoxel.y + 1 : m_CurrentVoxel.y) * cellSize,
            float((m_Step.z > 0) ? m_CurrentVoxel.z + 1 : m_CurrentVoxel.z) * cellSize
        );

        m_Max = (next_boundary - ray.Origin) / cellSize; // we will move along the axis with the smallest value
        m_Delta = cellSize / ray.Direction * glm::vec3(m_Step);
	}
    void Raymarch::Step()
    {
		if (m_Max.x < m_Max.y && m_Max.x < m_Max.z)
		{
			m_HitNormal = glm::vec3(float(-m_Step.x), 0.0, 0.0);
			m_Max.x += m_Delta.x;
			m_CurrentVoxel.x += m_Step.x;
		}
		else if (m_Max.y < m_Max.z)
		{
			m_HitNormal = glm::vec3(0.0, float(-m_Step.y), 0.0);
			m_Max.y += m_Delta.y;
			m_CurrentVoxel.y += m_Step.y;
		}
		else
		{
			m_HitNormal = glm::vec3(0.0, 0.0, float(-m_Step.z));
			m_Max.z += m_Delta.z;
			m_CurrentVoxel.z += m_Step.z;
		}
		m_DistanceTraveled += m_Size * glm::length(m_Ray.Direction);
    }
}