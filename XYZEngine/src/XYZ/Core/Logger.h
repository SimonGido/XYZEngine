#pragma once
#include "Core.h"
#include "XYZ/Core/Ref/Ref.h"
#include "XYZ/Core/Ref/WeakRef.h"


// This ignores all warnings raised inside External headers
#pragma warning(push, 0)


#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace XYZ {


	class XYZ_API Logger
	{
	public:
		enum Level
		{
			TraceLevel		= SPDLOG_LEVEL_TRACE,
			DebugLevel		= SPDLOG_LEVEL_DEBUG,
			InfoLevel		= SPDLOG_LEVEL_INFO,
			WarnLevel		= SPDLOG_LEVEL_WARN,
			ErrorLevel		= SPDLOG_LEVEL_ERROR,
			CriticalLevel	= SPDLOG_LEVEL_CRITICAL,
			Off				= SPDLOG_LEVEL_OFF,
			Num
		};

		Logger(std::string name, Level level = Level::TraceLevel, std::string file = "");

		template <typename ...Args>
		void Trace(fmt::format_string<Args...> fmt, Args &&...args)
		{
			std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
			m_SpdLogger->trace(msg);
		}

		template <typename ...Args>
		void Info(fmt::format_string<Args...> fmt, Args &&...args)
		{
			std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
			m_SpdLogger->info(msg);
		}

		template <typename ...Args>
		void Warn(fmt::format_string<Args...> fmt, Args &&...args)
		{
			std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
			m_SpdLogger->warn(msg);
		}

		template <typename ...Args>
		void Error(fmt::format_string<Args...> fmt, Args &&...args)
		{
			std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
			m_SpdLogger->error(msg);
		}

		template <typename ...Args>
		void Critical(fmt::format_string<Args...> fmt, Args &&...args)
		{
			std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
			m_SpdLogger->critical(msg);
		}

		const std::shared_ptr<spdlog::logger>& GetSpdLogger() const { return m_SpdLogger; }

	private:
		std::shared_ptr<spdlog::logger> m_SpdLogger;
	};


	class XYZ_API CoreLogger
	{
	public:
		static void Init();


		template <typename ...Args>
		static void Trace(fmt::format_string<Args...> fmt, Args &&...args)
		{
			std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
			trace(msg.c_str());
		}

		template <typename ...Args>
		static void Info(fmt::format_string<Args...> fmt, Args &&...args)
		{
			std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
			info(msg.c_str());
		}

		template <typename ...Args>
		static void Warn(fmt::format_string<Args...> fmt, Args &&...args)
		{
			std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
			warn(msg.c_str());
		}
		
		template <typename ...Args>
		static void Error(fmt::format_string<Args...> fmt, Args &&...args)
		{
			std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
			error(msg.c_str());
		}

		template <typename ...Args>
		static void Critical(fmt::format_string<Args...> fmt, Args &&...args)
		{
			std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
			critical(msg.c_str());
		}


		template <typename ...Args>
		static void TraceClient(fmt::format_string<Args...> fmt, Args &&...args)
		{
			std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
			traceClient(msg.c_str());
		}

		template <typename ...Args>
		static void InfoClient(fmt::format_string<Args...> fmt, Args &&...args)
		{
			std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
			infoClient(msg.c_str());
		}

		template <typename ...Args>
		static void WarnClient(fmt::format_string<Args...> fmt, Args &&...args)
		{
			std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
			warnClient(msg.c_str());
		}

		template <typename ...Args>
		static void ErrorClient(fmt::format_string<Args...> fmt, Args &&...args)
		{
			std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
			errorClient(msg.c_str());
		}

		template <typename ...Args>
		static void CriticalClient(fmt::format_string<Args...> fmt, Args &&...args)
		{
			std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
			criticalClient(msg.c_str());
		}



	private:
		static void trace(const char* msg);
		static void info(const char* msg);
		static void warn(const char* msg);
		static void error(const char* msg);
		static void critical(const char* msg);

		static void traceClient(const char* msg);
		static void infoClient(const char* msg);
		static void warnClient(const char* msg);
		static void errorClient(const char* msg);
		static void criticalClient(const char* msg);
	};


	// Core log macros
	#define XYZ_CORE_TRACE(...)    ::XYZ::CoreLogger::Trace(__VA_ARGS__)
	#define XYZ_CORE_INFO(...)     ::XYZ::CoreLogger::Info(__VA_ARGS__)
	#define XYZ_CORE_WARN(...)     ::XYZ::CoreLogger::Warn(__VA_ARGS__)
	#define XYZ_CORE_ERROR(...)    ::XYZ::CoreLogger::Error(__VA_ARGS__)
	#define XYZ_CORE_CRITICAL(...) ::XYZ::CoreLogger::Critical(__VA_ARGS__)

	// Client log macros
	#define XYZ_TRACE(...)         ::XYZ::CoreLogger::TraceClient(__VA_ARGS__)
	#define XYZ_INFO(...)          ::XYZ::CoreLogger::InfoClient(__VA_ARGS__)
	#define XYZ_WARN(...)          ::XYZ::CoreLogger::WarnClient(__VA_ARGS__)
	#define XYZ_ERROR(...)         ::XYZ::CoreLogger::ErrorClient(__VA_ARGS__)
	#define XYZ_CRITICAL(...)      ::XYZ::CoreLogger::CriticalClient(__VA_ARGS__)
}