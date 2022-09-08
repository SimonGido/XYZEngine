#pragma once
#include "Core.h"

#include "UDPConnection.h"

#include "XYZ/Utils/DataStructures/Queue.h"

namespace XYZ {

	class UDPClient
	{
	public:
		UDPClient(asio::io_context& asioContext);

		bool Connect(const std::string_view host, const uint16_t port);
		void Send(const std::string& data);

	private:
		std::shared_ptr<UDPConnection> m_Connection;
		asio::ip::udp::socket		   m_Socket;
		asio::io_context&			   m_Context;

		Queue<std::string>			   m_MessagesOut;
	};
}