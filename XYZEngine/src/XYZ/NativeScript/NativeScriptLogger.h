#pragma once
#include <RuntimeCompiler/ICompilerLogger.h>

namespace XYZ {
	const size_t LOGSYSTEM_MAX_BUFFER = 4096;

	class NativeScriptLogger : public ICompilerLogger
	{
	public:
		virtual void LogError(const char* format, ...) override;
		virtual void LogWarning(const char* format, ...) override;
		virtual void LogInfo(const char* format, ...) override;

	private:
		void LogInternal(const char* format, va_list args);
		char m_buff[LOGSYSTEM_MAX_BUFFER];
	};
}