#include "ClientLayer.h"

namespace XYZ {

	void ClientLayer::OnAttach()
	{
		m_Client.Connect("192.168.7.179", 60000);
	}
	void ClientLayer::OnDetach()
	{
		m_Client.Disconnect();
	}
	void ClientLayer::OnUpdate(Timestep ts)
	{
	}
	void ClientLayer::OnEvent(Event& event)
	{
	}
}
