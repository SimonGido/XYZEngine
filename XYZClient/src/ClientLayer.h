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

	class CustomClient : public Net::Client<MessageType>
	{
	public:
		CustomClient();

		void PingServer()
		{
			Net::Message<MessageType> msg;
			msg.Header.ID = MessageType::ServerPing;

			std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
			msg << timeNow;
			Send(msg);
		}

		void Update(Timestep ts);
		void UpdatePlayers(Net::Message<MessageType>& message);
		void OnEvent(Event& event);
		void SetID(uint32_t id) { m_ID = id; }
	private:

		std::vector<Player>			 m_Players;
		uint32_t					 m_ID;
		static constexpr float		 sc_Speed = 5.0f;
	};

	class ClientLayer : public Layer
	{
	public:
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;

	private:
		CustomClient m_Client;
		
	};
}