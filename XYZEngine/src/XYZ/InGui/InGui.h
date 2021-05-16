#pragma once
#include "InGuiContext.h"



namespace XYZ {

	class InGui
	{
	public:
		static void BeginFrame();
		static void EndFrame();

		static bool Begin(const char* name);

		static void End();
		
		static InGuiContext& GetContext();
	};
}