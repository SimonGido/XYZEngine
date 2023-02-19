#include "stdafx.h"
#include "XYZ/Core/Platform.h"

#include <array>

namespace XYZ {
	void Platform::RunShellCommand(std::string app, const std::string& args)
	{
        std::replace(app.begin(), app.end(), '/', '\\');

        STARTUPINFOA si;
        PROCESS_INFORMATION pi;

        // set the size of the structures
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        si.wShowWindow = SW_SHOW;
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.lpTitle = strdup(app.c_str());
       
        LPSTR argsTemp = strdup(args.c_str());
        BOOL success = CreateProcessA(
            app.c_str(),
            argsTemp,
            NULL,
            NULL,
            FALSE,
            CREATE_NEW_CONSOLE,
            NULL,
            NULL,
            &si,
            &pi
        );
        if (!success)
        {
            XYZ_CORE_ERROR("Failed to run shell command {}", app);
        }
        // Close process and thread handles. 
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        free(si.lpTitle);
        free(argsTemp);
	}
}