#pragma once
#include <algorithm>
#include <iostream>
#include <optional>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>

namespace XYZ {

	class Stopwatch
	{
	public:
		Stopwatch()
		{
			m_Start = std::chrono::high_resolution_clock::now();
		}

		~Stopwatch()
		{
			m_End = std::chrono::high_resolution_clock::now();

			auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_Start).time_since_epoch().count();
			auto end = std::chrono::time_point_cast<std::chrono::microseconds>(m_End).time_since_epoch().count();
			float ms = (end - start) * 0.001f;
			std::cout << "Time: " << ms << "ms" << " FPS: " << 1000.0f / ms << std::endl;
		}

	private:
		using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

		TimePoint m_Start, m_End;
	};

}