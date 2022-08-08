#pragma once

#include <spdlog/sinks/base_sink.h>
#include <mutex>

namespace XYZ {
	namespace Editor {

		struct ConsoleMessage
		{
			enum class Category { None  = -1, Info = BIT(0), Warning = BIT(1), Error = BIT(2)};

			ConsoleMessage() = default;
			ConsoleMessage(const std::string& msg, Category category)
				: Message(msg), MessageCategory(category)
			{}
			std::string Message;
			Category	MessageCategory = Category::None;
		};

		struct MessageStream
		{
			MessageStream(std::deque<ConsoleMessage>* messageBuffer, uint32_t messageLimit)
				:
				m_MessageBuffer(messageBuffer),
				m_MessageLimit(messageLimit)
			{
				XYZ_ASSERT(messageBuffer != nullptr, "");
			}
			MessageStream& operator<<(const ConsoleMessage& message)
			{
				m_MessageBuffer->push_back(message);
				if (static_cast<uint32_t>(m_MessageBuffer->size()) > m_MessageLimit)
					m_MessageBuffer->pop_front();

				return *this;
			}

		private:
			std::deque<ConsoleMessage>* m_MessageBuffer;
			uint32_t					m_MessageLimit;
		};

		class EditorConsoleSink : public spdlog::sinks::base_sink<std::mutex>
		{
		public:
			explicit EditorConsoleSink(MessageStream stream, uint32_t bufferCapacity);

			virtual ~EditorConsoleSink() = default;

			EditorConsoleSink(const EditorConsoleSink& other) = delete;
			EditorConsoleSink& operator=(const EditorConsoleSink& other) = delete;

		protected:
			virtual void sink_it_(const spdlog::details::log_msg& msg) override;
			virtual void flush_() override;
			

		private:
			static ConsoleMessage::Category getMessageCategory(spdlog::level::level_enum level);
			

		private:
			uint32_t					m_MessageBufferCapacity;
			std::vector<ConsoleMessage> m_MessageBuffer;
			uint32_t					m_MessageCount = 0;
			MessageStream				m_Stream;
		};
	}
}