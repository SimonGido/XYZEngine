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

	class CustomServer : public Net::Server<MessageType>
	{
	public:
		CustomServer(uint16_t port);
			

	protected:
		virtual bool onClientConnect(std::shared_ptr<Net::Connection<MessageType>> client) override;
	
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