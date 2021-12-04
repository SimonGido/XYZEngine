#pragma once
#include "Core.h"
#include "XYZ/Core/Ref/Ref.h"
#include "XYZ/Core/Ref/WeakRef.h"

#include <fstream>
#include <iostream>

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace XYZ {

	class Logger
	{
	public:
		static void Init();

		static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};


	// Core log macros
	#define XYZ_CORE_TRACE(...)    ::XYZ::Logger::GetCoreLogger()->trace(__VA_ARGS__)
	#define XYZ_CORE_INFO(...)     ::XYZ::Logger::GetCoreLogger()->info(__VA_ARGS__)
	#define XYZ_CORE_WARN(...)     ::XYZ::Logger::GetCoreLogger()->warn(__VA_ARGS__)
	#define XYZ_CORE_ERROR(...)    ::XYZ::Logger::GetCoreLogger()->error(__VA_ARGS__)
	#define XYZ_CORE_CRITICAL(...) ::XYZ::Logger::GetCoreLogger()->critical(__VA_ARGS__)

	// Client log macros
	#define XYZ_TRACE(...)         ::XYZ::Logger::GetClientLogger()->trace(__VA_ARGS__)
	#define XYZ_INFO(...)          ::XYZ::Logger::GetClientLogger()->info(__VA_ARGS__)
	#define XYZ_WARN(...)          ::XYZ::Logger::GetClientLogger()->warn(__VA_ARGS__)
	#define XYZ_ERROR(...)         ::XYZ::Logger::GetClientLogger()->error(__VA_ARGS__)
	#define XYZ_CRITICAL(...)      ::XYZ::Logger::GetClientLogger()->critical(__VA_ARGS__)
}