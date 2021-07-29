#pragma once

#include <XYZ.h>

#include "Player.h"

namespace XYZ {

	enum class MessageType : uint32_t
	{
		ServerAccept,
		ServerDeny,
		ServerPing,
		MessageAll,
		ServerMessage,
		PlayerUpdate
	};

	class CustomServer : public Net::Server<MessageType>
	{
	public:
		CustomServer(uint16_t port);
			

	protected:
		virtual void onClientConnect(std::shared_ptr<Net::Connection<MessageType>> client) override;
		virtual void onClientDisconnect(std::shared_ptr<Net::Connection<MessageType>> client);
		virtual void onMessage(std::shared_ptr<Net::Connection<MessageType>> client, Net::Message<MessageType>& msg) override;
	
	private:
		std::vector<Player> m_Players;
	};

	class ServerLayer : public Layer
	{
	public:
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;

	private:
		CustomServer* m_Server;		
	};
}