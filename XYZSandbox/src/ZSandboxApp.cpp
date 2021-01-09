#include <XYZ.h>
#include <XYZ/Core/EntryPoint.h>

#include "GameLayer.h"

namespace XYZ {
	class Sandbox : public XYZ::Application
	{
	public:
		Sandbox()
		{
			PushLayer(new GameLayer());
		}

		~Sandbox()
		{

		}
	};
}
XYZ::Application* CreateApplication()
{
	return new XYZ::Sandbox();
}
