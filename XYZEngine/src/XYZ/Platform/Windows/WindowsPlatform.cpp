
#include "stdafx.h"
#include "XYZ/Core/Platform.h"

#ifdef XYZ_PLATFORM_WINDOWS

#include <windows.h>
#include <atlstr.h>

#include <array>

namespace XYZ {
 
    bool Platform::ExecuteCommand(std::string app, const std::string& args, std::string& output)
    {
        const int bufferSize = 1024;
        char buffer[bufferSize];
        FILE* pipe;

        std::replace(app.begin(), app.end(), '/', '\\');
        std::string fullCommand = app + " " + args;

        pipe = _popen(fullCommand.c_str(), "r");
        if (pipe == nullptr)
            return false;

        while (fgets(buffer, bufferSize, pipe))
        {
            output += buffer;
        }

        int endOfFileVal = feof(pipe);
        int closeReturnVal = _pclose(pipe); // Close pipe

        if (!endOfFileVal)
        {
            XYZ_CORE_ERROR("Failed to read the pipe to the end");
            return false;
        }     
        return true;
    }
}
#endif