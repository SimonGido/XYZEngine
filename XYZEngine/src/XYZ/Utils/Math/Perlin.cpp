#include "stdafx.h"
#include "Perlin.h"


#include "PerlinNoise.hpp"

namespace XYZ {

	static siv::PerlinNoise s_Noise(rand());

	void Perlin::SetSeed(uint32_t seed)
	{
		s_Noise.reseed(seed);
	}

	double Perlin::Octave2D(double x, double y, uint32_t octaves)
	{
		return s_Noise.octave2D_01(x, y, octaves);
	}
	
}