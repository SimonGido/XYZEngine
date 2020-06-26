#include <XYZ.h>
#include <XYZ/Core/EntryPoint.h>

#include "GameLayer.h"
#include "Editor/Editor.h"

class Sandbox : public XYZ::Application
{
public:
	Sandbox()
	{
		PushLayer(new GameLayer());
		PushLayer(new Editor());
	}

	~Sandbox()
	{

	}
};

XYZ::Application* CreateApplication()
{
	return new Sandbox();
}
