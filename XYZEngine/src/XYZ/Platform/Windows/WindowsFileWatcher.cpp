#include "stdafx.h"
#include "WindowsFileWatcher.h"

#include <future> 

#ifdef XYZ_PLATFORM_WINDOWS

#include <windows.h>
namespace XYZ {

	static void FileWatcherThread(std::shared_ptr<FileWatcher> watcherObj)
	{
		const HANDLE hDir = CreateFile(watcherObj->GetDirectory().c_str(),  // pointer to the file name
		                               FILE_LIST_DIRECTORY,									 // access (read/write) mode
		                               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,  // share mode
		                               NULL,													 // security descriptor
		                               OPEN_EXISTING,											 // how to create
		                               FILE_FLAG_BACKUP_SEMANTICS,								 // file attributes
		                               NULL													 // file with attributes to copy
		);

		wchar_t filename[MAX_PATH];
		FILE_NOTIFY_INFORMATION Buffer[1024];
		DWORD BytesReturned;

		while (ReadDirectoryChangesW(
			hDir,                                   // handle to directory
			&Buffer,                                // read results buffer
			sizeof(Buffer),                         // length of buffer
			TRUE,									// monitoring option
			FILE_NOTIFY_CHANGE_SECURITY |
			FILE_NOTIFY_CHANGE_CREATION |
			FILE_NOTIFY_CHANGE_LAST_ACCESS |
			FILE_NOTIFY_CHANGE_LAST_WRITE |
			FILE_NOTIFY_CHANGE_SIZE |
			FILE_NOTIFY_CHANGE_ATTRIBUTES |
			FILE_NOTIFY_CHANGE_DIR_NAME |
			FILE_NOTIFY_CHANGE_FILE_NAME,            // filter conditions
			&BytesReturned,							 // bytes returned
			NULL,									 // overlapped buffer
			NULL									 // completion routine
		) && watcherObj->IsRunning())
		{
			const int offset = 0;
			FILE_NOTIFY_INFORMATION* pNotify;
			pNotify = (FILE_NOTIFY_INFORMATION*)((char*)Buffer + offset);
			wcscpy(filename, L"");

			wcsncpy(filename, pNotify->FileName, pNotify->FileNameLength / 2);

			filename[pNotify->FileNameLength / 2] = NULL;

			auto fullPath = watcherObj->GetDirectory() + L"/" + filename;
			while (true)
			{	
				switch (Buffer[0].Action)
				{
				case FILE_ACTION_MODIFIED:
					watcherObj->OnFileChange(filename);
					break;
				case FILE_ACTION_ADDED:
					watcherObj->OnFileAdded(filename);
					break;
				case FILE_ACTION_REMOVED:
					watcherObj->OnFileRemoved(filename);
					break;
				case FILE_ACTION_RENAMED_OLD_NAME:
					watcherObj->OnFileRenamed(filename);
					break;
				case FILE_ACTION_RENAMED_NEW_NAME:
					watcherObj->OnFileRenamed(filename);
					break;
				}
				break;			
			}		
		}
		CloseHandle(hDir);
	}

	WindowsFileWatcher::WindowsFileWatcher(const std::wstring& dir)
		:
		FileWatcher(dir)
	{
	}

	void WindowsFileWatcher::Start()
	{
		m_FileWatcherThread = std::unique_ptr<std::thread>(new std::thread(FileWatcherThread, this->shared_from_this()));
		m_Running = true;
	}

	void WindowsFileWatcher::Stop()
	{
		m_Running = false;
		m_FileWatcherThread->join();
	}

	std::shared_ptr<FileWatcher> FileWatcher::Create(const std::wstring& dir)
	{
		return std::make_shared<WindowsFileWatcher>(dir);
	}
}

#endif