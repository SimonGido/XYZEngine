#pragma once
#include "Core.h"

#include "XYZ/Utils/DataStructures/ThreadQueue.h"

namespace XYZ {

	class UDPClient : public std::enable_shared_from_this<UDPClient>
	{
	public:
		UDPClient(asio::io_context& asioContext, size_t recBufferSize = 65536);

		bool Connect(const std::string_view host, const uint16_t port);

		std::string Receive(asio::ip::udp::endpoint& endpoint, size_t size);
		void		ReceiveAsync();

		void Send(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size);
		bool SendAsync(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size);

		const asio::ip::udp::endpoint& GetEndpoint() const { return m_Endpoint; }
	protected:
		virtual void onConnected() {}

		virtual void onReceived(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size) = 0;

		virtual void onSent(const asio::ip::udp::endpoint& endpoint, size_t size) = 0;

		virtual void onError(std::error_code ec) = 0;

	private:
		size_t receive(asio::ip::udp::endpoint& endpoint, void* buffer, size_t size);

		void tryReceive();

	private:
		std::vector<std::byte>	 m_ReceiveBuffer;
		std::vector<std::byte>   m_SendBuffer;

		asio::io_context&		 m_Context;
		asio::ip::udp::endpoint  m_Endpoint;
		asio::ip::udp::endpoint  m_ReceiveEndpoint;
		asio::ip::udp::socket	 m_Socket;

		
		bool m_Connected;
		bool m_AsyncReceiving;
		bool m_AsyncSending;
	};
}