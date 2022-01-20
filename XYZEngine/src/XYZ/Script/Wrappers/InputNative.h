#pragma once

#include "XYZ/Core/Input.h"


namespace XYZ {
	namespace Script {

		struct InputNative
		{
			static void Register();
			static bool IsKeyPressed(KeyCode key);
		};
	}
}