#include "stdafx.h"
#include "MarchingCubes.h"

#include "Constants.h"


#include "Perlin.h"

namespace XYZ {
    void MarchingCubes::PerlinPolygonize(double isoLevel, const glm::vec3& min, const glm::vec3& max, uint32_t numCellsX, uint32_t numCellsY, uint32_t numCellsZ, std::vector<Triangle>& triangles)
    {
        const float width  = max.x - min.x;
        const float height = max.y - min.y;
        const float depth  = max.z - min.z;
      
        const float cellWidth  = width / (float)numCellsX;
        const float cellHeight = height / (float)numCellsY;
        const float cellDepth  = depth / (float)numCellsZ;
        const int32_t octaves = 5;
        const double frequency = 32.0;

        siv::PerlinNoise perlin{340 };
        for (uint32_t i = 0; i < numCellsX - 1; ++i)
        {
            for (uint32_t j = 0; j < numCellsY - 1; ++j)
            {
                for (uint32_t k = 0; k < numCellsZ - 1; ++k)
                {
                    double x = i; double y = j; double z = k;
                    GridCell cell;
                    cell.Point[0] = min + glm::vec3((x)        * cellWidth, (y)        * cellHeight, (z)        * cellDepth);
                    cell.Point[1] = min + glm::vec3((x + 1.0f) * cellWidth, (y)        * cellHeight, (z)        * cellDepth);
                    cell.Point[2] = min + glm::vec3((x + 1.0f) * cellWidth, (y)        * cellHeight, (z + 1.0f) * cellDepth);
                    cell.Point[3] = min + glm::vec3((x)        * cellWidth, (y)        * cellHeight, (z + 1.0f) * cellDepth);
                    cell.Point[4] = min + glm::vec3((x)        * cellWidth, (y + 1.0f) * cellHeight, (z)        * cellDepth);
                    cell.Point[5] = min + glm::vec3((x + 1.0f) * cellWidth, (y + 1.0f) * cellHeight, (z)        * cellDepth);
                    cell.Point[6] = min + glm::vec3((x + 1.0f) * cellWidth, (y + 1.0f) * cellHeight, (z + 1.0f) * cellDepth);
                    cell.Point[7] = min + glm::vec3((x)        * cellWidth, (y + 1.0f) * cellHeight, (z + 1.0f) * cellDepth);
                
                    x -= 0.5;
                    y -= 0.5;
                    z -= 0.5;

                    cell.Value[0] = perlin.noise3D(x,        y      ,z       );
                    cell.Value[1] = perlin.noise3D(x + 1.0,  y      ,z       );
                    cell.Value[2] = perlin.noise3D(x + 1.0,  y      ,z + 1.0f);
                    cell.Value[3] = perlin.noise3D(x,        y      ,z + 1.0f);
                    cell.Value[4] = perlin.noise3D(x,        y + 1.0,z       );
                    cell.Value[5] = perlin.noise3D(x + 1.0,  y + 1.0,z       );
                    cell.Value[6] = perlin.noise3D(x + 1.0,  y + 1.0,z + 1.0f);
                    cell.Value[7] = perlin.noise3D(x,        y + 1.0,z + 1.0f);
                
                    std::array<Triangle, 5> cellTriangles;
                    const size_t count = Polygonize(cell, isoLevel, cellTriangles);
                    for (size_t i = 0; i < count; ++i)
                        triangles.push_back(cellTriangles[i]);
                }
            }
        }
    }
    size_t MarchingCubes::Polygonize(const GridCell& cell, double isoLevel, std::array<Triangle, 5>& triangles)
    {
        uint8_t cubeIndex = findIndex(cell, isoLevel);

        if (sc_EdgeTable[cubeIndex] == 0)
            return 0;

        glm::vec3 vertices[12];
        for (uint32_t i = 0; i < 12; ++i)
        {
            if (sc_EdgeTable[cubeIndex] & (1 << i))
            {
                const size_t indexA = sc_EdgeConnection[i][0];
                const size_t indexB = sc_EdgeConnection[i][1];
                vertices[i] = interpolateVertex(
                    isoLevel,
                    cell.Point[indexA], cell.Point[indexB],
                    cell.Value[indexA], cell.Value[indexB]
                );
                vertices[i] = glm::vec3(0.5f);
            }
        }

        size_t ntriang = 0;
        for (int i = 0; sc_TriTable[cubeIndex][i] != -1; i += 3) 
        {
            triangles[ntriang][0] = vertices[sc_TriTable[cubeIndex][i]];
            triangles[ntriang][1] = vertices[sc_TriTable[cubeIndex][i + 1]];
            triangles[ntriang][2] = vertices[sc_TriTable[cubeIndex][i + 2]];
            ntriang++;
        }

        return ntriang;
    }
    uint8_t MarchingCubes::findIndex(const GridCell& cell, double isoLevel)
    {
        uint8_t result = 0;
        for (uint8_t i = 0; i < 8; ++i)
        {
            if (cell.Value[i] < isoLevel)
                result |= 1 << i;
        }
        return result;
    }
    glm::vec3 MarchingCubes::interpolateVertex(double isoLevel, const glm::vec3& p0, const glm::vec3& p1, double val0, double val1)
    {
        constexpr double treshold = 0.000001;

        float t = (isoLevel - val0) / (val1 - val0);
        return p0 + (t * (p1 - p0));
    }
}