#include "stdafx.h"
#include "UDPConnection.h"

namespace XYZ {
    UDPClient::UDPClient(asio::io_context& asioContext, asio::ip::udp::endpoint endpoint, asio::ip::udp::socket& socket)
        :
        m_Context(asioContext),
        m_Endpoint(endpoint),
        m_Socket(socket)
    {
        
    }

    void UDPClient::Send(const std::string& message)
    {
        if (m_Socket.is_open())
        {
            bool writing = !m_MessagesOut.Empty();
            m_MessagesOut.PushBack(message);

            if (!writing)
            {
                sendMessages();
            }
        }
    }

    void UDPClient::Receive()
    {
        m_Socket.async_receive_from(
            asio::buffer(m_Buffer),
            m_ReceiveEndpoint,
            std::bind(&UDPClient::handleReceive, shared_from_this(), std::placeholders::_1, std::placeholders::_2)
        );
    }

    void UDPClient::sendMessages()
    {      
        m_Socket.async_send_to(
            asio::buffer(m_MessagesOut.Front()),
            m_Endpoint,
            std::bind(&UDPClient::handleSend, shared_from_this(), std::placeholders::_1, std::placeholders::_2)
        );   
    }
    void UDPClient::handleSend(const std::error_code& ec, std::size_t length)
    {
        if (!ec)
        {
            m_MessagesOut.PopFront();
            if (!m_MessagesOut.Empty())
            {
                sendMessages();
            }
        }
    }
    void UDPClient::handleReceive(const std::error_code& ec, std::size_t length)
    {
        XYZ_INFO("Received {}", length);
        Receive();
    }
}