#pragma once
#include "Logger.h"
#include "Application.h"




extern XYZ::Application* CreateApplication();

/** Application entry point */
int main(int argc, char** argv)
{
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	auto app = CreateApplication();
	app->Run();
	delete app;
	_CrtDumpMemoryLeaks();
	XYZ::Audio::ShutDown();
}
