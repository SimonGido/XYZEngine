#pragma once
#include "NetMessage.h"
#include "NetConnection.h"

namespace XYZ {
	namespace Net {
		template <typename T>
		class Client
		{
		public:
			Client()
				: m_Socket(m_Context)
			{}
			virtual ~Client()
			{
				Disconnect();
			}

			bool Connect(const std::string& host, const uint16_t port)
			{
				try
				{
					asio::ip::tcp::resolver resolver(m_Context);
					auto endpoints = resolver.resolve(host, std::to_string(port));

					m_Connection = std::make_unique<Connection<T>>(
						Connection<T>::Owner::Client,
						m_Context,
						asio::ip::tcp::socket(m_Context),
						m_MessagesIn
					);
					
					m_Connection->ConnectToServer(endpoints);
					m_ContextThread = std::thread([this]() {m_Context.run(); });
				}
				catch (std::exception& e)
				{
					// TODO: Logging
					XYZ_CORE_ERROR("Client Exception: ", e.what());
					return false;
				}
				return true;
			}

			void Disconnect()
			{
				if (IsConnected())
				{
					m_Connection->Disconnect();
				}
				m_Context.stop();
				if (m_ContextThread.joinable())
					m_ContextThread.join();
			}

			void Send(const Message<T>& msg)
			{
				if (IsConnected())
					m_Connection->Send(msg);
			}

			bool IsConnected() const
			{
				if (m_Connection)
					return m_Connection->IsConnected();
				return false;
			}

			Queue<OwnedMessage<T>>& GetIncomingMessages() { return m_MessagesIn; }

		protected:
			asio::io_context m_Context;

			std::thread m_ContextThread;

			asio::ip::tcp::socket m_Socket;

			std::unique_ptr<Connection<T>> m_Connection;

		private:
			Queue<OwnedMessage<T>> m_MessagesIn;
		};
	}
}