#pragma once

#include <XYZ.h>

#include "XYZ/Net/UDPServer.h"
#include "XYZ/Net/UDPClient.h"


#include "Player.h"

namespace XYZ {

	
	class ServerLayer : public Layer
	{
	public:
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;

	private:
		std::shared_ptr<UDPServer> m_Server;	
		std::vector<UDPClient*>    m_Clients;

		asio::io_context m_AsioContext;

		std::thread m_AsioThread;
	};
}