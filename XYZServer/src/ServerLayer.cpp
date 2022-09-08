#include "ServerLayer.h"

namespace XYZ {

	void ServerLayer::OnAttach()
	{
		try
		{
			m_Server = std::make_shared<UDPServer>(m_AsioContext, 60000);

			m_Server->Start();
			XYZ_INFO("UDP Server started at port {}", 60000);

			const size_t numClients = 1;
			for (size_t i = 0; i < numClients; ++i)
			{
				m_Clients.push_back(new UDPClient(m_AsioContext));
				m_Clients.back()->Connect("192.168.0.227", 60000);
				m_Clients.back()->Receive();
			}


			m_AsioThread = std::thread([this]() { m_AsioContext.run(); });			
		}
		catch (std::exception& e)
		{

		}
	}
	void ServerLayer::OnDetach()
	{
		m_Server->Stop();
	}

	float timePassed = 0.0f;

	void ServerLayer::OnUpdate(Timestep ts)
	{
		timePassed += ts;
		if (timePassed > 1.0f)
		{
			std::string message = "Hello World!";
			for (size_t i = 0; i < m_Clients.size(); ++i)
			{
				m_Clients[i]->Send(message + std::to_string(i));
			}
			timePassed = 0.0f;
		}
	}
	void ServerLayer::OnEvent(Event& event)
	{
		if (event.GetEventType() == EventType::KeyPressed)
		{
			
		}
	}
	
}
