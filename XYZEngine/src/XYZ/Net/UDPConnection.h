#pragma once
#include "Core.h"

#include "XYZ/Utils/DataStructures/Queue.h"

namespace XYZ {

	class UDPConnection : public std::enable_shared_from_this<UDPConnection>
	{
	public:
		UDPConnection(asio::io_context& asioContext, asio::ip::udp::endpoint endpoint, asio::ip::udp::socket& socket);


		void Send(const std::string& message);
		
	private:
		void sendMessages();
		void handleSend(const std::error_code& ec, std::size_t length);

	private:
		asio::io_context&		 m_Context;
		asio::ip::udp::endpoint  m_Endpoint;
		asio::ip::udp::socket&	 m_Socket;
		Queue<std::string>		 m_MessagesOut;
	};
}