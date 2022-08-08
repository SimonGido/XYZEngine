#include "stdafx.h"
#include "EditorLogger.h"


namespace XYZ {
	namespace Editor {

		std::shared_ptr<spdlog::logger> EditorLogger::s_Logger;

		void EditorLogger::Init(MessageStream stream)
		{
			std::vector<spdlog::sink_ptr> logSinks;

			logSinks.emplace_back(std::make_shared<EditorConsoleSink>(stream, 1));
			s_Logger = std::make_shared<spdlog::logger>("Editor", begin(logSinks), end(logSinks));
			spdlog::register_logger(s_Logger);
			s_Logger->set_level(spdlog::level::trace);
			s_Logger->flush_on(spdlog::level::trace);
		}
	}
}