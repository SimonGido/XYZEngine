#pragma once

#include "Queue.h"
#include "NetMessage.h"

namespace XYZ {
	namespace Net {

		template <typename T>
		class Connection : public std::enable_shared_from_this<Connection<T>>
		{
		public:
			enum class Owner
			{
				Server,
				Client
			};

			Connection(Owner owner, asio::io_context& asioContext, asio::ip::tcp::socket socket, Queue<OwnedMessage<T>>& inMessages)
				: m_Owner(owner), m_AsioContext(asioContext), m_Socket(std::move(socket)), m_MessagesIn(inMessages)
			{

			}

			virtual ~Connection()
			{

			}

			void Send(const Message<T>& msg)
			{
				asio::post(m_AsioContext, [this, msg] {

					// If queue is not empty it is not writing message , it has no task so start new task ( writeHeader )
					bool writingMessage = !m_MessagesOut.Empty();
					m_MessagesOut.PushBack(msg);
					if (!writingMessage)
						writeHeader();
				});
			}

			void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
			{
				if (m_Owner == Owner::Client)
				{
					asio::async_connect(m_Socket, endpoints,
						[this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {

							if (!ec)
							{
								readHeader();
							}
						});
				}
			}

			void ConnectToClient(uint32_t id)
			{
				if (m_Owner == Owner::Server)
				{
					if (m_Socket.is_open())
					{
						m_ID = id;
						readHeader();
					}
				}
			}

			void Disconnect()
			{
				if (IsConnected())
					asio::post(m_AsioContext, [this]() { m_Socket.close(); });
			}

			bool IsConnected() const
			{
				return m_Socket.is_open();
			}

			uint32_t GetID() const
			{
				return m_ID;
			}
		private:
	
			void readHeader()
			{			
				asio::async_read(m_Socket, asio::buffer(&m_TemporaryMessage.Header, sizeof(MessageHeader<T>)),
					[this](std::error_code ec, std::size_t length) {
					std::cout << "Reading header " << m_TemporaryMessage.Header.Size << std::endl;
 						if (!ec)
						{
							if (m_TemporaryMessage.Header.Size > 0)
							{
								m_TemporaryMessage.Body.resize(m_TemporaryMessage.Header.Size);
								readBody();
							}
							else
							{
								addToIncomingMessageQueue();
							}
						}
						else
						{
							XYZ_LOG_ERR("[", m_ID, "]", " Read header failed");
							m_Socket.close();
						}
					});
			}
			void readBody()
			{			
				asio::async_read(m_Socket, asio::buffer(m_TemporaryMessage.Body.data(), m_TemporaryMessage.Header.Size),
					[this](std::error_code ec, std::size_t length) {
					std::cout << "Reading body " << m_TemporaryMessage.Body.size() << std::endl;
						if (!ec)
						{
							addToIncomingMessageQueue(); 						
						}
						else
						{
							XYZ_LOG_ERR("[", m_ID, "]", " Read body failed");
							m_Socket.close();
						}
					
					});
			}

			void writeHeader()
			{
				asio::async_write(m_Socket, asio::buffer(&m_MessagesOut.Front().Header, sizeof(MessageHeader<T>)),
					[this](std::error_code ec, std::size_t length) {
						if (!ec)
						{
							if (m_MessagesOut.Front().Body.size() > 0)
							{
								writeBody();
							}
							else
							{
								m_MessagesOut.PopFront();
								if (!m_MessagesOut.Empty())
									writeHeader();
							}
						}
						else
						{
							XYZ_LOG_ERR("[", m_ID, "]", " Write header failed");
							m_Socket.close();
						}
					});
			}

			void writeBody()
			{
				asio::async_write(m_Socket, asio::buffer(m_MessagesOut.Front().Body.data(), m_MessagesOut.Front().Body.size()),
					[this](std::error_code ec, std::size_t length) {
						if (!ec)
						{
							m_MessagesOut.PopFront();
							if (!m_MessagesOut.Empty())
								writeHeader();
						}
						else
						{
							XYZ_LOG_ERR("[", m_ID, "]", " Write body failed");
							m_Socket.close();
						}
					});
			}

			void addToIncomingMessageQueue()
			{
				std::cout << "Add incoming" << std::endl;
				if (m_Owner == Owner::Server)
					m_MessagesIn.PushBack({ this->shared_from_this(), m_TemporaryMessage });
				else
					m_MessagesIn.PushBack({ nullptr, m_TemporaryMessage });	

				readHeader();
			}

		private:
			Owner m_Owner;

			Message<T> m_TemporaryMessage;

			asio::io_context& m_AsioContext;

			asio::ip::tcp::socket m_Socket;
			
			Queue<OwnedMessage<T>>& m_MessagesIn;

			Queue<Message<T>> m_MessagesOut;

			uint32_t m_ID = 0;
		};
	}
}