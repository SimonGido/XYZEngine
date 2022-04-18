#pragma once
#include <random>

namespace XYZ {
	class InterpolatedNoise
	{
	public:
		float getNoise(int x, int z)
		{
			std::mt19937 rng(x + z + m_Seed);
			std::uniform_real_distribution<> dist(-1, 1);

			return dist(rng);
		}

		float getSmoothNoise(int x, int z)
		{
			float corners = (getNoise(x - 1, z - 1) + getNoise(x + 1, z - 1) + getNoise(x - 1, z + 1)
				+ getNoise(x + 1, z + 1)) / 16.0f;
			float sides = (getNoise(x - 1, z) + getNoise(x + 1, z) + getNoise(x, z - 1)
				+ getNoise(x, z + 1)) / 8.0f;
			float center = getNoise(x, z) / 4.0f;

			return corners + sides + center;
		}

		float getInterpolatedNoise(float x, float z)
		{
			int intX = (int)x;
			int intZ = (int)z;
			float fracX = x - intX;
			float fracZ = z - intZ;


			float v1 = getSmoothNoise(intX, intZ);
			float v2 = getSmoothNoise(intX + 1, intZ);
			float v3 = getSmoothNoise(intX, intZ + 1);
			float v4 = getSmoothNoise(intX + 1, intZ + 1);
			float i1 = interpolateCosine(v1, v2, fracX);
			float i2 = interpolateCosine(v3, v4, fracX);


			return interpolateCosine(i1, i2, fracZ);
		}

		float interpolateCosine(float a, float b, float blend)
		{
			double theta = blend * 3.14;
			float f = (float)(1.0f - cos(theta)) * 0.5f;
			return a * (1.0f - f) + b * f;
		}

		float generateHeight(int x, int z)
		{
			float total = 0;
			float d = (float)pow(2, m_Octaves - 1);
			for (int i = 0; i < m_Octaves; i++) {
				float freq = (float)(pow(2, i) / d);
				float amp = (float)pow(m_Roughness, i) * m_Amplitude;
				total += getInterpolatedNoise((x)*freq, (z * freq) * amp);
			}
			return total;
		}

		int m_Seed = 2;
		int m_Octaves = 4;
		float m_Roughness = 2;
		float m_Amplitude = 4;
	};
}