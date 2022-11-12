#include "stdafx.h"
#include "Logger.h"


// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

#include <ozz/base/log.h>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace XYZ {
	static std::shared_ptr<spdlog::logger> s_CoreLogger;
	static std::shared_ptr<spdlog::logger> s_ClientLogger;

	void Logger::Init()
	{
		// turn off ozz logging
		ozz::log::SetLevel(ozz::log::kSilent);

		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("XYZ.log", true));

		logSinks[0]->set_pattern("%^[%T] %n: %v%$");
		logSinks[1]->set_pattern("[%T] [%l] %n: %v");

		s_CoreLogger = std::make_shared<spdlog::logger>("XYZ", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_CoreLogger);
		s_CoreLogger->set_level(spdlog::level::trace);
		s_CoreLogger->flush_on(spdlog::level::trace);

		s_ClientLogger = std::make_shared<spdlog::logger>("APP", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_ClientLogger);
		s_ClientLogger->set_level(spdlog::level::trace);
		s_ClientLogger->flush_on(spdlog::level::trace);
	}
	void Logger::trace(const std::string& msg)
	{
		s_CoreLogger->trace(msg);
	}
	void Logger::info(const std::string& msg)
	{
		s_CoreLogger->info(msg);
	}
	void Logger::warn(const std::string& msg)
	{
		s_CoreLogger->warn(msg);
	}
	void Logger::error(const std::string& msg)
	{
		s_CoreLogger->error(msg);
	}
	void Logger::critical(const std::string& msg)
	{
		s_CoreLogger->critical(msg);
	}
	void Logger::traceClient(const std::string& msg)
	{
		s_ClientLogger->trace(msg);
	}
	void Logger::infoClient(const std::string& msg)
	{
		s_ClientLogger->info(msg);
	}
	void Logger::warnClient(const std::string& msg)
	{
		s_ClientLogger->warn(msg);
	}
	void Logger::errorClient(const std::string& msg)
	{
		s_ClientLogger->error(msg);
	}
	void Logger::criticalClient(const std::string& msg)
	{
		s_ClientLogger->critical(msg);
	}
}


