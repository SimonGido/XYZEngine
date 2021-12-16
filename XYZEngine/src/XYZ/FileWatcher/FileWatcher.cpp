#include "stdafx.h"
#include "FileWatcher.h"


namespace XYZ {
	FileWatcher::FileWatcher(const std::wstring& dir)
		:
		m_Directory(dir)
	{
	}
	void FileWatcher::AddListener(FileWatcherListener* listener)
	{
		m_Listeners.push_back(listener);
	}
	void FileWatcher::OnFileChange(const std::wstring& fileName)
	{
		for (const auto listener : m_Listeners)
			listener->OnFileChange(fileName);
	}
	void FileWatcher::OnFileAdded(const std::wstring& fileName)
	{
		for (const auto listener : m_Listeners)
			listener->OnFileAdded(fileName);
	}
	void FileWatcher::OnFileRemoved(const std::wstring& fileName)
	{
		for (const auto listener : m_Listeners)
			listener->OnFileRemoved(fileName);
	}
	void FileWatcher::OnFileRenamed(const std::wstring& fileName)
	{
		for (const auto listener : m_Listeners)
			listener->OnFileRenamed(fileName);
	}
	
}