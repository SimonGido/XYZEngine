
#include <XYZ.h>


struct RCCppMainLoop : RCCppMainLoopI, TInterface<XYZ::IID_IRCCPP_MAIN_LOOP, IObject>
{
	RCCppMainLoop()
	{
		PerModuleInterface::g_pSystemTable->MainLoop = this;
	}

	void MainLoop() override
	{
		//PerModuleInterface::g_pSystemTable->CurrentScene->OnUpdate(0.0f);
	}
};


REGISTERSINGLETON(RCCppMainLoop, true);