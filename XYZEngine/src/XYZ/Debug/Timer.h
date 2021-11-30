#pragma once
#include <algorithm>
#include <iostream>
#include <optional>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>

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
		float Elapsed()
		{
			m_End = std::chrono::high_resolution_clock::now();

			const auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_Start).time_since_epoch().count();
			const auto end = std::chrono::time_point_cast<std::chrono::microseconds>(m_End).time_since_epoch().count();
			const float ms = (end - start) * 0.001f;
			return ms;
		}

	private:
		using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

		TimePoint m_Start, m_End;
	};


	class PerformanceProfiler
	{
	public:
		void PushMeasurement(const char* name, float value)
		{
			std::scoped_lock lock(m_DataMutex);
			m_PerFrameData[name] = value;
		}
		void LockData()
		{
			m_DataMutex.lock();
			m_IsLocked = true;
		}
		void UnlockData()
		{
			m_DataMutex.unlock();
			m_IsLocked = false;
		}
		const std::unordered_map<const char*, float>& GetPerformanceData() const
		{
			XYZ_ASSERT(m_IsLocked, "Data must be locked before access");
			return m_PerFrameData;
		}

	private:
		std::unordered_map<const char*, float> m_PerFrameData;
		std::mutex				 m_DataMutex;
		bool				     m_IsLocked = false;
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