#include "ServerLayer.h"

namespace XYZ {
	CustomServer::CustomServer(uint16_t port)
		: Net::Server<MessageType>(port)
	{}

	bool CustomServer::onClientConnect(std::shared_ptr<Net::Connection<MessageType>> client)
	{
		return true;
	}

	void CustomServer::onMessage(std::shared_ptr<Net::Connection<MessageType>> client, Net::Message<MessageType>& msg)
	{
		if (msg.Header.ID == MessageType::ServerPing)
		{
			std::cout << "Server pinged" << std::endl;
			client->Send(msg);
		}
	}

	void ServerLayer::OnAttach()
	{
		Application::Get().GetImGuiLayer()->BlockEvents(false);

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
		if (event.GetEventType() == EventType::KeyPressed)
		{
			Net::Message<MessageType> msg;
			msg.Header.ID = MessageType::MessageAll;
			msg.Header.Size = 0;
			m_Server->MessageAllClients(msg);
		}
	}
	
}
