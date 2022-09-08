#include "stdafx.h"
#include "UDPClient.h"

namespace XYZ {
	UDPClient::UDPClient(asio::io_context& asioContext)
		:
		m_Socket(asioContext),
		m_Context(asioContext)
	{
		
	}
	bool UDPClient::Connect(const std::string_view host, const uint16_t port)
	{
		try
		{
			asio::ip::udp::resolver resolver(m_Context);
			auto serverEndpoint = *resolver.resolve(asio::ip::udp::v4(), host, std::to_string(port)).begin();
			
			m_Socket.open(asio::ip::udp::v4());
			//m_Socket.connect(serverEndpoint);

			m_Connection = std::make_shared<UDPConnection>(m_Context, serverEndpoint, m_Socket);
		}
		catch (std::exception& e)
		{
			return false;
		}
		return true;
	}

	void UDPClient::Send(const std::string& data)
	{
		m_Connection->Send(data);
	}

	void UDPClient::Receive()
	{
		m_Connection->Receive();
	}

}