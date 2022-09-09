#include "stdafx.h"
#include "UDPClient.h"

namespace XYZ {
    UDPClient::UDPClient(asio::io_context& asioContext, size_t recBufferSize)
        :
        m_Context(asioContext),
        m_Socket(asioContext),
        m_Connected(false),
		m_AsyncReceiving(false),
		m_AsyncSending(false)
    {
		m_ReceiveBuffer.resize(recBufferSize);
    }

    bool UDPClient::Connect(const std::string_view host, const uint16_t port)
    {
        if (m_Connected)
            return false;

        asio::ip::udp::resolver resolver(m_Context);
        m_Endpoint = asio::ip::udp::endpoint(asio::ip::make_address(host), (unsigned short)port);
        m_Socket.open(m_Endpoint.protocol());
		//m_Socket.bind(m_Endpoint);
        m_Connected = true;
        onConnected();
        
        return true;
    }

    std::string UDPClient::Receive(asio::ip::udp::endpoint& endpoint, size_t size)
    {
		std::string message(size, 0);
		receive(endpoint, message.data(), message.size());
		return message;
    }

    void UDPClient::ReceiveAsync()
    {
		tryReceive();
    }

    void UDPClient::Send(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size)
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

    bool UDPClient::SendAsync(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size)
    {
		if (m_AsyncSending || size == 0)
			return false;

		m_AsyncSending = true;
		const std::byte* bytes = (const std::byte*)buffer;
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


	size_t UDPClient::receive(asio::ip::udp::endpoint& endpoint, void* buffer, size_t size)
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

	void UDPClient::tryReceive()
	{
		if (m_AsyncReceiving)
			return;

		m_AsyncReceiving = true;

		m_Socket.async_receive_from(
			asio::buffer(m_ReceiveBuffer),
			m_ReceiveEndpoint,
			[self = shared_from_this()](std::error_code ec, size_t read) {
			
			self->m_AsyncReceiving = false;
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