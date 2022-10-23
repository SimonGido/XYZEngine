#include "stdafx.h"
#include "AssimpLog.h"


static std::mutex s_Mutex;

void LogStream::Initialize()
{
	std::unique_lock lock(s_Mutex);
	if (Assimp::DefaultLogger::isNullLogger())
	{
		Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
		Assimp::DefaultLogger::get()->attachStream(new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
	}
}

void LogStream::write(const char* message)
{
	XYZ_CORE_ERROR("Assimp error: {0}", message);
}