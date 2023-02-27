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

	Logger::Logger(std::string name, Level level, std::string file)
	{
		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.back()->set_pattern("%^[%T] %n: %v%$");
		if (!file.empty())
		{
			logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(file, true));
			logSinks.back()->set_pattern("[%T] [%l] %n: %v");
		}

		m_SpdLogger = std::make_shared<spdlog::logger>(std::move(name), begin(logSinks), end(logSinks));
		spdlog::register_logger(m_SpdLogger);
		m_SpdLogger->set_level((spdlog::level::level_enum)level);
		m_SpdLogger->flush_on(spdlog::level::trace);
	}


	static std::shared_ptr<spdlog::logger> s_CoreLogger;
	static std::shared_ptr<spdlog::logger> s_ClientLogger;

	void CoreLogger::Init()
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
	void CoreLogger::trace(const char* msg)
	{
		s_CoreLogger->trace(msg);
	}
	void CoreLogger::info(const char* msg)
	{
		s_CoreLogger->info(msg);
	}
	void CoreLogger::warn(const char* msg)
	{
		s_CoreLogger->warn(msg);
	}
	void CoreLogger::error(const char* msg)
	{
		s_CoreLogger->error(msg);
	}
	void CoreLogger::critical(const char* msg)
	{
		s_CoreLogger->critical(msg);
	}
	void CoreLogger::traceClient(const char* msg)
	{
		s_ClientLogger->trace(msg);
	}
	void CoreLogger::infoClient(const char* msg)
	{
		s_ClientLogger->info(msg);
	}
	void CoreLogger::warnClient(const char* msg)
	{
		s_ClientLogger->warn(msg);
	}
	void CoreLogger::errorClient(const char* msg)
	{
		s_ClientLogger->error(msg);
	}
	void CoreLogger::criticalClient(const char* msg)
	{
		s_ClientLogger->critical(msg);
	}


}


