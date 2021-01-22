#pragma once
#include "Core.h"
#include "NetConnection.h"

namespace XYZ {
	namespace Net {

		template <typename T>
		class Server
		{
		public:
			Server(uint16_t port)
				: m_AsioAcceptor(m_AsioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
			{

			}

			virtual ~Server()
			{
				Stop();
			}

			bool Start()
			{
				try
				{
					WaitForClientConnection();
					m_ContextThread = std::thread([this]() { m_AsioContext.run();  });
				}
				catch (std::exception& e)
				{
					XYZ_LOG_ERR("Server Exception: ", e.what());
					return false;
				}
				XYZ_LOG_INFO("Server Started");
				return true;
			}

			void Stop()
			{
				m_AsioContext.stop();

				if (m_ContextThread.joinable())
					m_ContextThread.join();

				XYZ_LOG_INFO("Server Stopped");
			}

			void WaitForClientConnection()
			{
				m_AsioAcceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
					if (!ec)
					{
						XYZ_LOG_INFO("Server New Connection: ", socket.remote_endpoint());
						std::shared_ptr<Connection<T>> newConn =
							std::make_shared<Connection<T>>(Connection<T>::Owner::Server,
								m_AsioContext, std::move(socket), m_MessagesIn);
				
						if (onClientConnect(newConn))
						{
							m_Connections.push_back(std::move(newConn));
							uint32_t id = 0;
							if (!m_FreeClientIDs.empty())
							{
								id = m_FreeClientIDs.back();
								m_FreeClientIDs.pop_back();
							}
							else
							{
								id = m_NextClientID++;
							}

							m_Connections.back()->ConnectToClient(id);
							XYZ_LOG_INFO("[", m_Connections.back()->GetID(), "] Connection Approved");
							
						}
						else
						{
							XYZ_LOG_WARN("Server Connection Denied");
						}
					}
					else
					{
						XYZ_LOG_ERR("Server New Connection Error: ", ec.message());
					}
					WaitForClientConnection();
				});
			}

			void MessageClient(std::shared_ptr<Connection<T>> client, const Message<T>& msg)
			{
				if (client && client->IsConnected())
				{
					client->Send(msg);
				}
				else
				{
					onClientDisconnect(client);
					if (client)
						m_FreeClientIDs.push_back(client->GetID());

					client.reset();
					m_Connections.erase(
						std::remove(m_Connections.begin(), m_Connections.end(), client), m_Connections.end()
					);
				}
			}

			void MessageAllClients(const Message<T>& msg, std::shared_ptr<Connection<T>> ignoredClient = nullptr)
			{
				bool invalidClientExists = false;
				for (auto& client : m_Connections)
				{
					if (client && client->IsConnected())
					{
						if (client != ignoredClient)
							client->Send(msg);
					}
					else
					{
						onClientDisconnect(client);
						if (client)
							m_FreeClientIDs.push_back(client->GetID());

						client.reset();
						invalidClientExists = true;
					}
				}
				if (invalidClientExists)
				{
					m_Connections.erase(
						std::remove(m_Connections.begin(), m_Connections.end(), nullptr), m_Connections.end()
					);
				}
			}

			void Update(size_t maxMessages = -1)
			{
				size_t messageCount = 0;
				while (messageCount < maxMessages && !m_MessagesIn.Empty())
				{
					auto msg = m_MessagesIn.PopFront();
					onMessage(msg.Remote, msg.Message);
					messageCount++;
				}
			}

		protected:

			virtual bool onClientConnect(std::shared_ptr<Connection<T>> client)
			{
				return true;
			}

			virtual void onClientDisconnect(std::shared_ptr<Connection<T>> client)
			{

			}

			virtual void onMessage(std::shared_ptr<Connection<T>> client, Message<T>& msg)
			{

			}

		protected:
			Queue<OwnedMessage<T>> m_MessagesIn;
			std::deque<std::shared_ptr<Connection<T>>> m_Connections;

			asio::io_context m_AsioContext;
			asio::ip::tcp::acceptor m_AsioAcceptor;

			std::thread m_ContextThread;

			uint32_t m_NextClientID = 0;

			std::vector<uint32_t> m_FreeClientIDs;
		};
	}
}