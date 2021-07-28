#include "ClientLayer.h"

namespace XYZ {

	void ClientLayer::OnAttach()
	{
		m_Client.Connect("192.168.7.179", 60000);
		Application::Get().GetImGuiLayer()->BlockEvents(false);
	}
	void ClientLayer::OnDetach()
	{
		m_Client.Disconnect();
	}
	void ClientLayer::OnUpdate(Timestep ts)
	{
		if (m_Client.IsConnected())
		{
			if (!m_Client.GetIncomingMessages().Empty())
			{
				auto msg = std::move(m_Client.GetIncomingMessages().PopFront().Message);
				switch (msg.Header.ID)
				{
				case MessageType::ServerPing:
				{
					std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point timeThen;
					msg >> timeThen;
					std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << std::endl;
				}
				}
			}
		}
	}
	void ClientLayer::OnEvent(Event& event)
	{
		if (event.GetEventType() == EventType::KeyPressed)
		{
			m_Client.PingServer();
		}
	}
}
