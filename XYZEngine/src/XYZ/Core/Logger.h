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
		static void Init();

		template <typename ...Args>
		static void Trace(fmt::format_string<Args...> fmt, Args &&...args)
		{
			trace(fmt::format(fmt, std::forward<Args>(args)...));
		}

		template <typename ...Args>
		static void Info(fmt::format_string<Args...> fmt, Args &&...args)
		{
			info(fmt::format(fmt, std::forward<Args>(args)...));
		}

		template <typename ...Args>
		static void Warn(fmt::format_string<Args...> fmt, Args &&...args)
		{
			warn(fmt::format(fmt, std::forward<Args>(args)...));
		}
		
		template <typename ...Args>
		static void Error(fmt::format_string<Args...> fmt, Args &&...args)
		{
			error(fmt::format(fmt, std::forward<Args>(args)...));
		}

		template <typename ...Args>
		static void Critical(fmt::format_string<Args...> fmt, Args &&...args)
		{
			critical(fmt::format(fmt, std::forward<Args>(args)...));
		}


		template <typename ...Args>
		static void TraceClient(fmt::format_string<Args...> fmt, Args &&...args)
		{
			traceClient(fmt::format(fmt, std::forward<Args>(args)...));
		}

		template <typename ...Args>
		static void InfoClient(fmt::format_string<Args...> fmt, Args &&...args)
		{
			infoClient(fmt::format(fmt, std::forward<Args>(args)...));
		}

		template <typename ...Args>
		static void WarnClient(fmt::format_string<Args...> fmt, Args &&...args)
		{
			warnClient(fmt::format(fmt, std::forward<Args>(args)...));
		}

		template <typename ...Args>
		static void ErrorClient(fmt::format_string<Args...> fmt, Args &&...args)
		{
			errorClient(fmt::format(fmt, std::forward<Args>(args)...));
		}

		template <typename ...Args>
		static void CriticalClient(fmt::format_string<Args...> fmt, Args &&...args)
		{
			criticalClient(fmt::format(fmt, std::forward<Args>(args)...));
		}
	private:
		static void trace(const std::string& msg);
		static void info(const std::string& msg);
		static void warn(const std::string& msg);
		static void error(const std::string& msg);
		static void critical(const std::string& msg);

		static void traceClient(const std::string& msg);
		static void infoClient(const std::string& msg);
		static void warnClient(const std::string& msg);
		static void errorClient(const std::string& msg);
		static void criticalClient(const std::string& msg);
	};


	// Core log macros
	#define XYZ_CORE_TRACE(...)    ::XYZ::Logger::Trace(__VA_ARGS__)
	#define XYZ_CORE_INFO(...)     ::XYZ::Logger::Info(__VA_ARGS__)
	#define XYZ_CORE_WARN(...)     ::XYZ::Logger::Warn(__VA_ARGS__)
	#define XYZ_CORE_ERROR(...)    ::XYZ::Logger::Error(__VA_ARGS__)
	#define XYZ_CORE_CRITICAL(...) ::XYZ::Logger::Critical(__VA_ARGS__)

	// Client log macros
	#define XYZ_TRACE(...)         ::XYZ::Logger::TraceClient(__VA_ARGS__)
	#define XYZ_INFO(...)          ::XYZ::Logger::InfoClient(__VA_ARGS__)
	#define XYZ_WARN(...)          ::XYZ::Logger::WarnClient(__VA_ARGS__)
	#define XYZ_ERROR(...)         ::XYZ::Logger::ErrorClient(__VA_ARGS__)
	#define XYZ_CRITICAL(...)      ::XYZ::Logger::CriticalClient(__VA_ARGS__)
}