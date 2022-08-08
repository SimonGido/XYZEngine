#include "stdafx.h"
#include "EditorConsoleSink.h"


namespace XYZ {
	namespace Editor {

		EditorConsoleSink::EditorConsoleSink(MessageStream stream, uint32_t bufferCapacity)
			:
			m_MessageBufferCapacity(bufferCapacity),
			m_MessageBuffer(bufferCapacity),
			m_Stream(stream)
		{
		}
		void EditorConsoleSink::sink_it_(const spdlog::details::log_msg& msg)
		{
			spdlog::memory_buf_t formatted;
			spdlog::sinks::base_sink<std::mutex>::formatter_->format(msg, formatted);
			m_MessageBuffer[m_MessageCount++] = ConsoleMessage(fmt::to_string(formatted), getMessageCategory(msg.level));

			if (m_MessageCount == m_MessageBufferCapacity)
				flush_();
		}
		void EditorConsoleSink::flush_()
		{
			for (uint32_t i = 0; i < m_MessageCount; ++i)
			{
				m_Stream << m_MessageBuffer[i];
			}
			m_MessageCount = 0;
		}
		ConsoleMessage::Category EditorConsoleSink::getMessageCategory(spdlog::level::level_enum level)
		{
			switch (level)
			{
			case spdlog::level::trace:
			case spdlog::level::debug:
			case spdlog::level::info:
				return ConsoleMessage::Category::Info;
			case spdlog::level::warn:
				return ConsoleMessage::Category::Warning;
			case spdlog::level::err:
			case spdlog::level::critical:
				return ConsoleMessage::Category::Error;
			}

			XYZ_ASSERT(false, "Invalid Message Category!");
			return ConsoleMessage::Category::None;
		}
	}
}