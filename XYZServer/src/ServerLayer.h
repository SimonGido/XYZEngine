#pragma once

#include <XYZ.h>

#include "XYZ/Net/UDPServer.h"
#include "XYZ/Net/UDPClient.h"


#include "Player.h"

namespace XYZ {


	class MyClient : public UDPClient
	{
	public:
		MyClient(asio::io_context& asioContext)
			:
			UDPClient(asioContext)
		{
		}
	protected:
		virtual void onReceived(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size) override
		{
			XYZ_INFO("Client received {} bytes", size);
		}

		virtual void onSent(const asio::ip::udp::endpoint& endpoint, size_t size) override
		{

		}

		virtual void onError(std::error_code ec) override
		{
			XYZ_INFO("Client Error: {}", ec.message());
		}
	};

	class MyServer : public UDPServer
	{
	public:
		MyServer(asio::io_context& asioContext, uint16_t port)
			:
			UDPServer(asioContext, port)
		{
		}
	protected:
		virtual void onReceived(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size) override
		{
			XYZ_INFO("Server received {} bytes", size);

			auto it = m_Clients.find(endpoint);
			if (it == m_Clients.end())
			{
				m_Clients.insert(endpoint);
			}
			for (const auto& client : m_Clients)
				SendAsync(client, buffer, size);
		}

		virtual void onSent(const asio::ip::udp::endpoint& endpoint, size_t size) override
		{

		}

		virtual void onError(std::error_code ec) override
		{
			XYZ_INFO("Server Error: {}", ec.message());
		}

	private:
		std::set<asio::ip::udp::endpoint> m_Clients;
	};
	
	class ServerLayer : public Layer
	{
	public:
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;

	private:
		std::shared_ptr<MyServer>			   m_Server;	
		std::vector<std::shared_ptr<MyClient>> m_Clients;

		asio::io_context m_AsioContext;

		std::thread m_AsioThread;
	};
}