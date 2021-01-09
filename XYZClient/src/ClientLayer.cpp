#include "ClientLayer.h"

namespace XYZ {

	void ClientLayer::OnAttach()
	{
		m_Client.Connect("147.32.121.43", 60000);
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
