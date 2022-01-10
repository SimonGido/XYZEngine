#include "stdafx.h"
#include "FileWatcher.h"


namespace XYZ {
	FileWatcher::FileWatcher(const std::wstring& dir)
		:
		m_Directory(dir),
		m_Running(false)
	{
	}
	FileWatcher::~FileWatcher()
	{
		Stop();
		for (auto listener : m_Listeners)
			delete listener;
	}
	void FileWatcher::Start()
	{
		m_Running = true;
		m_FileWatcherThread = std::make_unique<std::thread>(&FileWatcher::threadFunc, this->shared_from_this());
		m_FileWatcherThread->detach();
	}
	void FileWatcher::Stop()
	{
		m_Running = false;
	}
	bool FileWatcher::IsRunning()
	{
		return m_Running;
	}
	void FileWatcher::onFileChange(const std::wstring& fileName)
	{
		for (const auto listener : m_Listeners)
			listener->OnFileChange(fileName);
	}
	void FileWatcher::onFileAdded(const std::wstring& fileName)
	{
		for (const auto listener : m_Listeners)
			listener->OnFileAdded(fileName);
	}
	void FileWatcher::onFileRemoved(const std::wstring& fileName)
	{
		for (const auto listener : m_Listeners)
			listener->OnFileRemoved(fileName);
	}
	void FileWatcher::onFileRenamed(const std::wstring& fileName)
	{
		for (const auto listener : m_Listeners)
			listener->OnFileRenamed(fileName);
	}
}