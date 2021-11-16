#include <XYZ.h>
#include <XYZ/Core/EntryPoint.h>

#include "EditorLayer.h"


class Editor : public XYZ::Application
{
public:
	Editor()
		:XYZ::Application({})
	{
		PushLayer(new XYZ::EditorLayer());
	}

	~Editor()
	{

	}
};

XYZ::Application* CreateApplication()
{
	return new Editor();
}
