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
		std::scoped_lock lock(m_CallbacksMutex);
		for (auto& callable : m_OnFileChange)
			callable(fileName);
	}
	void FileWatcher::onFileAdded(const std::wstring& fileName)
	{
		std::scoped_lock lock(m_CallbacksMutex);
		for (auto& callable : m_OnFileAdded)
			callable(fileName);
	}
	void FileWatcher::onFileRemoved(const std::wstring& fileName)
	{
		std::scoped_lock lock(m_CallbacksMutex);
		for (auto& callable : m_OnFileRemoved)
			callable(fileName);
	}
	void FileWatcher::onFileRenamed(const std::wstring& fileName)
	{
		std::scoped_lock lock(m_CallbacksMutex);
		for (auto& callable : m_OnFileRenamed)
			callable(fileName);
	}
}