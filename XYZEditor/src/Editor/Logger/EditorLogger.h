#pragma once

#include "EditorConsoleSink.h"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace XYZ {
	namespace Editor {
		class EditorLogger
		{
		public:
			static void Init(MessageStream stream);

			static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }
		private:
			static std::shared_ptr<spdlog::logger> s_Logger;
		};


		// Editor log macros
		#define XYZ_EDITOR_TRACE(...)    ::XYZ::Editor::EditorLogger::GetLogger()->trace(__VA_ARGS__)
		#define XYZ_EDITOR_INFO(...)     ::XYZ::Editor::EditorLogger::GetLogger()->info(__VA_ARGS__)
		#define XYZ_EDITOR_WARN(...)     ::XYZ::Editor::EditorLogger::GetLogger()->warn(__VA_ARGS__)
		#define XYZ_EDITOR_ERROR(...)    ::XYZ::Editor::EditorLogger::GetLogger()->error(__VA_ARGS__)
		#define XYZ_EDITOR_CRITICAL(...) ::XYZ::Editor::EditorLogger::GetLogger()->critical(__VA_ARGS__)
	}
}