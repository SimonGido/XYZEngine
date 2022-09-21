#include <XYZ.h>
#include <XYZ/Core/EntryPoint.h>

#include "EditorLayer.h"


class Editor : public XYZ::Application
{
public:
	Editor()
		:
		XYZ::Application(XYZ::ApplicationSpecification{ true, true })
	{
		PushLayer(new XYZ::Editor::EditorLayer());
	}

	~Editor()
	{

	}
};

XYZ::Application* CreateApplication()
{
	return new Editor();
}
