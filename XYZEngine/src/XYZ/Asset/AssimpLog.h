#pragma once
#include "XYZ/Core/Logger.h"

#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

struct LogStream : public Assimp::LogStream
{
	static void Initialize()
	{
		if (Assimp::DefaultLogger::isNullLogger())
		{
			Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
			Assimp::DefaultLogger::get()->attachStream(new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
		}
	}

	virtual void write(const char* message) override
	{
		XYZ_ERROR("Assimp error: {0}", message);
	}
};