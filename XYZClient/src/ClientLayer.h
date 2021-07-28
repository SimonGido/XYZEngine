#pragma once

#include <XYZ.h>


namespace XYZ {
	enum class MessageType : uint32_t
	{
		ServerAccept,
		ServerDeny,
		ServerPing,
		MessageAll,
		ServerMessage
	};

	class CustomClient : public Net::Client<MessageType>
	{
	public:
		void PingServer()
		{
			Net::Message<MessageType> msg;
			msg.Header.ID = MessageType::ServerPing;

			std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
			msg << timeNow;
			Send(msg);
		}
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