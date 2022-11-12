#pragma once
#include "XYZ/Core/Logger.h"

#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>


#include <mutex>

struct XYZ_API LogStream : public Assimp::LogStream
{
	
	static void Initialize();
	

	virtual void write(const char* message) override;
	
};