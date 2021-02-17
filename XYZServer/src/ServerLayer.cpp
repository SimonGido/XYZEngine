#include "ServerLayer.h"

namespace XYZ {
	CustomServer::CustomServer(uint16_t port)
		: Net::Server<MessageType>(port)
	{}

	bool CustomServer::onClientConnect(std::shared_ptr<Net::Connection<MessageType>> client)
	{
		return true;
	}

	void ServerLayer::OnAttach()
	{
		m_Server = new CustomServer(60000);
		m_Server->Start();
	}
	void ServerLayer::OnDetach()
	{
		m_Server->Stop();
		delete m_Server;
	}
	void ServerLayer::OnUpdate(Timestep ts)
	{
		m_Server->Update();
	}
	void ServerLayer::OnEvent(Event& event)
	{
	}
	
}
