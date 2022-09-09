#include "ServerLayer.h"

namespace XYZ {

	void ServerLayer::OnAttach()
	{
		try
		{
			m_Server = std::make_shared<MyServer>(m_AsioContext, 60000);

			m_Server->Start();
			m_Server->ReceiveAsync();
			XYZ_INFO("UDP Server started at port {}", 60000);

			const size_t numClients = 1;
			for (size_t i = 0; i < numClients; ++i)
			{
				m_Clients.push_back(std::make_shared<MyClient>(m_AsioContext));
				m_Clients.back()->Connect("192.168.0.227", 60000);
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
		m_Server->ReceiveAsync();
		timePassed += ts;
		if (timePassed > 1.0f)
		{
			for (size_t i = 0; i < m_Clients.size(); ++i)
			{
				std::string message = "Hello World!" + std::to_string(i);
				m_Clients[i]->SendAsync(m_Clients[i]->GetEndpoint(), message.data(), message.size());
				m_Clients[i]->ReceiveAsync();
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
