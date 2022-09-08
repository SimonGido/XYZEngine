#include "stdafx.h"
#include "UDPServer.h"


namespace XYZ {
	UDPServer::UDPServer(asio::io_context& asioContext, uint16_t port)
		:
		m_Context(asioContext),
		m_Socket(m_Context, asio::ip::udp::endpoint(asio::ip::udp::v6(), port))
	{
	
	}
	void UDPServer::Start()
	{
		requestNextPacket();
	}
	void UDPServer::Stop()
	{
	}
	void UDPServer::requestNextPacket()
	{
		asio::ip::udp::endpoint remoteEndpoint;
		m_Socket.async_receive_from(asio::buffer(m_Buffer), remoteEndpoint, 
			[self = shared_from_this()](asio::error_code error, size_t length) {
				self->receive(error, length);
			});
	}
	void UDPServer::receive(asio::error_code error, size_t length)
	{		
		if (error)
		{
			if (error != asio::error::operation_aborted || error != asio::error::shut_down)
			{
				requestNextPacket();
			}
			return;
		}

		try
		{

		}
		catch (std::exception& e) 
		{
		}

		requestNextPacket();
	}

	void UDPServer::acceptConnection()
	{
	}
}