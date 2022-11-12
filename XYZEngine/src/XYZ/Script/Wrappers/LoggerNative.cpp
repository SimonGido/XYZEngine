#include "stdafx.h"
#include "LoggerNative.h"

#include "XYZ/Script/ScriptEngine.h"
#include "XYZ/Scene/Prefab.h"


#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace XYZ {
	namespace Script {
		void LoggerNative::Register()
		{
			mono_add_internal_call("XYZ.Log::Info_Native", Info);
			mono_add_internal_call("XYZ.Log::Warn_Native", Warn);
			mono_add_internal_call("XYZ.Log::Error_Native", Error);
		}
		void LoggerNative::Info(MonoString* message)
		{
			if (ScriptEngine::GetLogger())
				ScriptEngine::GetLogger()->info(mono_string_to_utf8(message));
		}
		void LoggerNative::Warn(MonoString* message)
		{
			if (ScriptEngine::GetLogger())
				ScriptEngine::GetLogger()->warn(mono_string_to_utf8(message));
		}
		void LoggerNative::Error(MonoString* message)
		{
			if (ScriptEngine::GetLogger())
				ScriptEngine::GetLogger()->error(mono_string_to_utf8(message));
		}
	}
}