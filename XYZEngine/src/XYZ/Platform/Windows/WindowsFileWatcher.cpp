#include "stdafx.h"
#include "XYZ/FileWatcher/FileWatcher.h"

#ifdef XYZ_PLATFORM_WINDOWS

#include <windows.h>

namespace XYZ {

	void FileWatcher::threadFunc(std::shared_ptr<FileWatcher> watcher)
	{
		const HANDLE hDir = CreateFile(watcher->m_Directory.c_str(),			 // pointer to the file name
			FILE_LIST_DIRECTORY,									 // access (read/write) mode
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,  // share mode
			NULL,													 // security descriptor
			OPEN_EXISTING,											 // how to create
			FILE_FLAG_BACKUP_SEMANTICS,								 // file attributes
			NULL													 // file with attributes to copy
		);

		wchar_t filename[MAX_PATH];
		FILE_NOTIFY_INFORMATION buffer[64];
		DWORD bytesReturned;

		const DWORD notifyFlags =
			FILE_NOTIFY_CHANGE_SECURITY |
			FILE_NOTIFY_CHANGE_CREATION |
			FILE_NOTIFY_CHANGE_LAST_ACCESS |
			FILE_NOTIFY_CHANGE_LAST_WRITE |
			FILE_NOTIFY_CHANGE_SIZE |
			FILE_NOTIFY_CHANGE_ATTRIBUTES |
			FILE_NOTIFY_CHANGE_DIR_NAME |
			FILE_NOTIFY_CHANGE_FILE_NAME;

		while (watcher->IsRunning() && ReadDirectoryChangesW(
			hDir,                                    // handle to directory
			&buffer,                                 // read results buffer
			sizeof(buffer),                          // length of buffer
			TRUE,									 // monitoring option
			notifyFlags,							 // filter conditions
			&bytesReturned,							 // bytes returned
			NULL,									 // overlapped buffer
			NULL									 // completion routine
		))
		{
			const int offset = 0;
			FILE_NOTIFY_INFORMATION* pNotify;
			pNotify = (FILE_NOTIFY_INFORMATION*)((char*)buffer + offset);
			wcscpy_s(filename, L"");

			wcsncpy_s(filename, pNotify->FileName, pNotify->FileNameLength / 2);

			filename[pNotify->FileNameLength / 2] = NULL;

			auto fullPath = watcher->m_Directory + L"/" + filename;
			while (true)
			{
				switch (buffer[0].Action)
				{
				case FILE_ACTION_MODIFIED:
					watcher->onFileChange(filename);
					break;
				case FILE_ACTION_ADDED:
					watcher->onFileAdded(filename);
					break;
				case FILE_ACTION_REMOVED:
					watcher->onFileRemoved(filename);
					break;
				case FILE_ACTION_RENAMED_OLD_NAME:
					watcher->onFileRenamed(filename);
					break;
				case FILE_ACTION_RENAMED_NEW_NAME:
					watcher->onFileRenamed(filename);
					break;
				}
				break;
			}
		}
		CloseHandle(hDir);
	}
}

#endif