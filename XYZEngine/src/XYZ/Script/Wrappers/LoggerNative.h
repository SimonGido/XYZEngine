#pragma once

#include "XYZ/Script/ScriptWrappers.h"


namespace XYZ {
	namespace Script {

		struct LoggerNative
		{
			static void Register();
			static void Info(MonoString* message);
			static void Warn(MonoString* message);
			static void Error(MonoString* message);
		};
	}
}