#include "stdafx.h"
#include "InputNative.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace XYZ {
	namespace Script {
		void InputNative::Register()
		{
			mono_add_internal_call("XYZ.Input::IsKeyPressed_Native", IsKeyPressed);

		}
		bool InputNative::IsKeyPressed(KeyCode key)
		{
			return Input::IsKeyPressed(key);
		}
	}
}