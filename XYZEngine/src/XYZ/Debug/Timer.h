#pragma once
#include <algorithm>
#include <iostream>
#include <optional>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>

#include "XYZ/Utils/DataStructures/ThreadPass.h"

namespace XYZ {

	class Scopewatch
	{
	public:
		Scopewatch()
		{
			m_Start = std::chrono::high_resolution_clock::now();
		}
		~Scopewatch()
		{
			m_End = std::chrono::high_resolution_clock::now();

			const auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_Start).time_since_epoch().count();
			const auto end = std::chrono::time_point_cast<std::chrono::microseconds>(m_End).time_since_epoch().count();
			const float ms = (end - start) * 0.001f;
			std::cout << "Time: " << ms << "ms" << " FPS: " << 1000.0f / ms << std::endl;
		}
	private:
		using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

		TimePoint m_Start, m_End;
	};

	class Stopwatch
	{
	public:
		Stopwatch()
		{
			m_Start = std::chrono::high_resolution_clock::now();
		}

		void Restart()
		{
			m_Start = std::chrono::high_resolution_clock::now();
		}

		float Elapsed()
		{
			m_End = std::chrono::high_resolution_clock::now();

			const auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_Start).time_since_epoch().count();
			const auto end = std::chrono::time_point_cast<std::chrono::microseconds>(m_End).time_since_epoch().count();
			const float ms = (end - start) * 0.001f;
			return ms;
		}

		float ElapsedSeconds()
		{
			return Elapsed() * 0.001f;
		}

	private:
		using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

		TimePoint m_Start, m_End;
	};


	class PerformanceProfiler
	{
	public:
		using PerformanceMap = std::unordered_map<const char*, float>;

		void PushMeasurement(const char* name, float value)
		{
			std::unique_lock lock(m_DataMutex);	
			m_PerFrameData[name] = value;
		}

		ScopedLockRead<PerformanceMap> GetPerformanceData() const
		{
			return ScopedLockRead<PerformanceMap>(&m_DataMutex, m_PerFrameData);
		}

	private:
		PerformanceMap			   m_PerFrameData;
		mutable std::shared_mutex  m_DataMutex;
	};


	class ScopePerfTimer
	{
	public:
		ScopePerfTimer(const char* name, PerformanceProfiler& profiler)
			: m_Name(name), m_Profiler(profiler) {}

		~ScopePerfTimer()
		{
			float time = m_Timer.Elapsed();
			m_Profiler.PushMeasurement(m_Name, time );
		}
	private:
		const char*			 m_Name;
		PerformanceProfiler& m_Profiler;
		Stopwatch			 m_Timer;
	};

	#define XYZ_SCOPE_PERF(name)\
	ScopePerfTimer timer__LINE__(name, Application::Get().GetPerformanceProfiler())
}