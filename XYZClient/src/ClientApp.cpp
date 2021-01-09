#include <XYZ.h>
#include <XYZ/Core/EntryPoint.h>

#include "ClientLayer.h"


class ClientApp : public XYZ::Application
{
public:
	ClientApp()
	{
		PushLayer(new XYZ::ClientLayer());
	}

};

XYZ::Application* CreateApplication()
{
	return new ClientApp();
}
