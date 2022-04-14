#pragma once

#include <XYZ.h>




namespace XYZ {

	using Triangle = std::array<glm::vec3, 3>;

	struct GridCell
	{
		glm::vec3 Point[8];
		double	  Value[8];
	};

	class MarchingCubes
	{
	public:
		static void PerlinPolygonize(double isoLevel, const glm::vec3& min, const glm::vec3& max, uint32_t a, uint32_t b, uint32_t c, std::vector<Triangle>& triangles);

		static size_t Polygonize(const GridCell& cell, double isoLevel, std::array<Triangle, 5>& triangles);



	private:
		static uint8_t findIndex(const GridCell& cell, double isoLevel);
		static glm::vec3 interpolateVertex(double isoLevel, const glm::vec3& p0, const glm::vec3& p1, double val0, double val1);
	};
}