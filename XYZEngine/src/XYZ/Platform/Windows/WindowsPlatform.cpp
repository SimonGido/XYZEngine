#include "stdafx.h"
#include "XYZ/Core/Platform.h"

#include <array>

namespace XYZ {
	std::string Platform::RunShellCommand(const char* command)
	{
        std::array<char, 128> buffer;
        std::string result;
   
        FILE* pipe = _popen(command, "r");
        if (pipe) 
        {
            XYZ_CORE_ERROR("Failed to run shell command {}", command);
       
            while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) 
            {
                result += buffer.data();
            }
            _pclose(pipe);
        }
        return result;
	}
}