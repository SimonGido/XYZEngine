#pragma once
#include <algorithm>
#include <iostream>
#include <optional>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>

#ifdef CORE_PLATFORM_WINDOWS
#include <Windows.h>
#endif // CORE_PLATFORM_WINDOWS

namespace XYZ {
	class Instrumentor
	{
		struct ProfileResult
		{
			std::string Name;
			long long Start, End;
			size_t ThreadID;
			size_t ProcessID;
		};

		struct Session
		{
			std::string Name;
		};

		Session* m_CurrentSession;
		std::ofstream m_OutputStream;
		size_t m_ProfileCount;

		void WriteHeader()
		{
			m_OutputStream << "{\"otherData\": {}, \"traceEvents\":[";
			m_OutputStream.flush();
		}
		void WriteFooter()
		{
			m_OutputStream << "]}";
			m_OutputStream.flush();
		}
		Instrumentor()
			: m_CurrentSession(nullptr), m_ProfileCount(0)
		{
			const std::string filepath = "results.json";
			const std::string name = "";

			m_OutputStream.open(filepath);
			WriteHeader();
			m_CurrentSession = new Session{ name };
		}
		~Instrumentor()
		{
			WriteFooter();
			m_OutputStream.close();
			delete m_CurrentSession;
			m_CurrentSession = nullptr;
			m_ProfileCount = 0;
		}
	public:
		static Instrumentor& Get()
		{
			static Instrumentor i;
			return i;
		}
		void WriteProfile(const ProfileResult& result)
		{
			if (m_ProfileCount++ > 0)
				m_OutputStream << ",";

			std::string name = result.Name;
			std::replace(name.begin(), name.end(), '"', '\'');

			m_OutputStream << "{";
			m_OutputStream << "\"cat\":\"function\",";
			m_OutputStream << "\"dur\":" << (result.End - result.Start) << ",";
			m_OutputStream << "\"name\":\"" << result.Name << "\",";
			m_OutputStream << "\"ph\":\"X\",";
			m_OutputStream << "\"pid\":" << result.ProcessID << ",";
			m_OutputStream << "\"tid\":" << result.ThreadID << ",";
			m_OutputStream << "\"ts\":" << result.Start;
			m_OutputStream << "}";

			m_OutputStream.flush();
		}
	};


	/*
		Basic scope timer.
		At begining of the sope construct object of Stopwatch (Stopwatch sw()).
		Name of scope can be passed in constructor. Function/lambda can be also  passed in constructor that will be passed args of const std::string, const float args.
		const String [0] -> Scope name, const float [1] -> time in ms.
		Function will be called on scope end.
		After object goes out of scope result's will be written to std::cout.

		results.json will be generated. This can be viewed in "chrome://tracing". Process id, thread id's are passed.
	*/
	class Stopwatch
	{
		template<typename T>
		using OptionalReference = std::optional<std::reference_wrapper<T>>;

		using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
		using OptionalFunction = OptionalReference<void(const std::string, const float)>; // referene_wrapper can reference function so no need for function.. + its broken
		//using OptionalFunction = OptionalReference<std::function<void(const std::string, const float)>>;
		//using OptionalString = OptionalReference<const std::string&>;

		std::string m_Name;
		TimePoint m_Start, m_End;
		OptionalFunction m_Function;

		size_t GetProcessID()
		{
#ifdef CORE_PLATFORM_WINDOWS
			return GetCurrentProcessId();
#endif // _CORE_WINDOWS
			return 0;
		}
	public:
		Stopwatch(const std::string& name = "", const OptionalFunction func = std::nullopt)
			: m_Name(name), m_Function(func)
		{
			m_Start = std::chrono::high_resolution_clock::now();
		}

		~Stopwatch()
		{
			m_End = std::chrono::high_resolution_clock::now();

			auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_Start).time_since_epoch().count();
			auto end = std::chrono::time_point_cast<std::chrono::microseconds>(m_End).time_since_epoch().count();
			float ms = (end - start) * 0.001f;

			if (m_Function) (m_Function.value())(m_Name, ms);

			size_t processID = GetProcessID();
			size_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
			Instrumentor::Get().WriteProfile({ m_Name, start, end, threadID, processID });

			std::cout << "Performance: " << m_Name << std::endl;
			std::cout << "Time: " << ms << "ms" <<" FPS: " << 1000.0f/ms << std::endl;
		}
	};

}