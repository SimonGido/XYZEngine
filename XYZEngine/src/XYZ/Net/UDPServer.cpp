#include "stdafx.h"
#include "UDPServer.h"


namespace XYZ {
	UDPServer::UDPServer(asio::io_context& asioContext, uint16_t port, size_t recBufferSize)
		:
		m_Context(asioContext),
		m_Socket(m_Context),
		m_Port(port),
		m_Running(false),
		m_AsyncReceiving(false),
		m_AsyncSending(false)
	{
		m_ReceiveBuffer.resize(recBufferSize);
	}
	void UDPServer::Start()
	{
		XYZ_ASSERT(!m_Running, "UDP Server already started!");

		m_Running = true;
		m_Socket.open(asio::ip::udp::endpoint(asio::ip::udp::v6(), m_Port).protocol());
		onStarted();
	}
	void UDPServer::Stop()
	{
		m_Running = false;
		m_Socket.close();
		onStopped();
	}
	std::string UDPServer::Receive(asio::ip::udp::endpoint& endpoint, size_t size)
	{
		std::string message(size, 0);
		receive(endpoint, message.data(), message.size());
		return message;
	}
	void UDPServer::ReceiveAsync()
	{
		tryReceive();
	}
	void UDPServer::Send(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size)
	{
		std::atomic_bool finished = false;

		m_Socket.async_send_to(
			asio::buffer(buffer, size), 
			endpoint, 
			[&, self = shared_from_this(), endpoint](asio::error_code ec, size_t size) {
				finished = true;
				if (ec)
				{
					self->onError(ec);
					self->onSent(endpoint, size);
					return;
				}
				self->onSent(endpoint, size);
			});
	}
	bool UDPServer::SendAsync(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size)
	{
		if (m_AsyncSending)
			return false;

		m_AsyncSending = true;
		const uint8_t* bytes = (const uint8_t*)buffer;
		m_SendBuffer.assign(bytes, bytes + size);


		m_Socket.async_send_to(
			asio::buffer(m_SendBuffer.data(), size), 
			endpoint, 
			[self = shared_from_this(), endpoint](asio::error_code ec, size_t size) {

				self->m_AsyncSending = false;
				if (ec)
				{
					self->onError(ec);
					self->onSent(endpoint, size);
					return;
				}
				self->onSent(endpoint, size);
			});

		return true;
	}


	size_t UDPServer::receive(asio::ip::udp::endpoint& endpoint, void* buffer, size_t size)
	{		
		std::atomic_bool finished = false;

		size_t received = 0;
		m_Socket.async_receive_from(
			asio::buffer(buffer, size), 
			endpoint, 
			[&](std::error_code ec, size_t size) 
			{ 
				received = size; 
				finished = true;
			});

		while (!finished) {};
		
		onReceived(endpoint, buffer, received);
		return received;
	}

	void UDPServer::tryReceive()
	{
		if (m_AsyncReceiving)
			return;

		m_AsyncReceiving = true;

		m_Socket.async_receive_from(
			asio::buffer(m_ReceiveBuffer),
			m_ReceiveEndpoint,
			[self = shared_from_this()](std::error_code ec, size_t read) {
			
				if (ec)
				{
					self->onError(ec);
					self->onReceived(self->m_ReceiveEndpoint, self->m_ReceiveBuffer.data(), read);
					return;
				}
				self->onReceived(self->m_ReceiveEndpoint, self->m_ReceiveBuffer.data(), read);

				if (self->m_ReceiveBuffer.size() == read)
				{
					self->onError(asio::error::no_buffer_space);
				}
			});
	}


}