#pragma once
#include "Core.h"

#include "UDPClient.h"


namespace XYZ {
	
	struct UDPServerConfiguration
	{

	};

	class UDPServer : public std::enable_shared_from_this<UDPServer>
	{
	public:
		UDPServer(asio::io_context& asioContext, uint16_t port, size_t recBufferSize = 65536);

		void Start();
		void Stop();

		std::string Receive(asio::ip::udp::endpoint& endpoint, size_t size);
		void		ReceiveAsync();

		void Send(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size);
		bool SendAsync(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size);

	protected:
		virtual void onStarted() {};

		virtual void onStopped() {};

		virtual void onReceived(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size) = 0;

		virtual void onSent(const asio::ip::udp::endpoint& endpoint, size_t size) = 0;
	
		virtual void onError(std::error_code ec) = 0;
	
	private:
		size_t receive(asio::ip::udp::endpoint& endpoint, void* buffer, size_t size);

		void tryReceive();

	private:
		std::vector<std::byte>	m_ReceiveBuffer;
		std::vector<std::byte>  m_SendBuffer;

		asio::io_context&	    m_Context;
		asio::ip::udp::socket   m_Socket;
		asio::ip::udp::endpoint m_ReceiveEndpoint;

		uint16_t m_Port;
		bool	 m_Running;
		bool	 m_AsyncReceiving;
		bool	 m_AsyncSending;
	};
}