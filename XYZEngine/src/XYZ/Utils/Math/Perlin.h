#pragma once


namespace XYZ {

	class Perlin
	{
	public:
		static void SetSeed(uint32_t seed);

		static double Octave2D(double x, double y, uint32_t octaves);

	};
}