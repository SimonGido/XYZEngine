#include "ServerLayer.h"

namespace XYZ {

	static void SerializePlayer(Net::Message<MessageType>& message, const Player& player)
	{
		message << player.Position;
		message << player.Size;
		message << player.Color;
		message << player.ID;
	}

	static Player DeserializePlayer(Net::Message<MessageType>& message)
	{
		Player player;
		message >> player.ID;
		message >> player.Color;
		message >> player.Size;
		message >> player.Position;
		return player;
	}

	CustomServer::CustomServer(uint16_t port)
		: Net::Server<MessageType>(port)
	{}

	void CustomServer::onClientConnect(std::shared_ptr<Net::Connection<MessageType>> client)
	{
		m_Players.push_back({});
		m_Players.back().ID = client->GetID();
		{
			Net::Message<MessageType> message;
			message.Header.ID = MessageType::ServerAccept;
			message << client->GetID();

			MessageClient(client, message);
		}
		{
			Net::Message<MessageType> message;
			message.Header.ID = MessageType::PlayerUpdate;

			for (auto& player : m_Players)
				SerializePlayer(message, player);
			message << m_Players.size();
			MessageAllClients(message);
		}
	}

	void CustomServer::onClientDisconnect(std::shared_ptr<Net::Connection<MessageType>> client)
	{
		for (size_t i = 0; i < m_Players.size(); ++i)
		{
			if (m_Players[i].ID == client->GetID())
			{
				m_Players.erase(m_Players.begin() + i);
				break;
			}
		}
		Net::Message<MessageType> message;
		message.Header.ID = MessageType::PlayerUpdate;

		
		for (auto& player : m_Players)
			SerializePlayer(message, player);

		message << m_Players.size();
		MessageAllClients(message);
	}

	void CustomServer::onMessage(std::shared_ptr<Net::Connection<MessageType>> client, Net::Message<MessageType>& msg)
	{
		if (msg.Header.ID == MessageType::ServerPing)
		{
			std::cout << "Server pinged" << std::endl;
			client->Send(msg);
		}
		if (msg.Header.ID == MessageType::PlayerUpdate)
		{
			Player player = DeserializePlayer(msg);
			m_Players[player.ID] = player;

			Net::Message<MessageType> message;
			message.Header.ID = MessageType::PlayerUpdate;
			
			SerializePlayer(message, player);
			message << (size_t)1;

			MessageAllClients(message, client);
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
