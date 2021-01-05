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

			bool Send(const Message<T>& msg)
			{
				return false;
			}

			bool ConnectToServer()
			{
				return false; 
			}

			void ConnectToClient(uint32_t id)
			{
				if (m_Owner == Owner::Server)
				{
					if (m_Socket.is_open())
					{
						m_ID = id;
					}
				}
			}

			bool Disconnect()
			{
				return false; 
			}

			bool IsConnected() const
			{
				return false;
			}

			uint32_t GetID() const
			{
				return m_ID;
			}

		private:
			Owner m_Owner;

			asio::io_context& m_AsioContext;

			asio::ip::tcp::socket m_Socket;
			
			Queue<OwnedMessage<T>>& m_MessagesIn;

			Queue<Message<T>> m_MessagesOut;

			uint32_t m_ID;
		};
	}
}