#include "stdafx.h"
#include "FileWatcher.h"


namespace XYZ {
	FileWatcher::FileWatcher(const std::filesystem::path& dir)
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
	void FileWatcher::ProcessChanges()
	{
		while (!m_FileChanges.Empty())
		{
			Change change = std::move(m_FileChanges.PopFront());
			for (auto& callback : m_OnFileChange)
				callback(change.Type, change.FilePath);
		}
	}
	void FileWatcher::onFileModified(const std::filesystem::path& fileName)
	{
		m_FileChanges.PushBack({ ChangeType::Modified, fileName });
	}
	void FileWatcher::onFileAdded(const std::filesystem::path& fileName)
	{
		m_FileChanges.PushBack({ ChangeType::Added, fileName });
	}
	void FileWatcher::onFileRemoved(const std::filesystem::path& fileName)
	{
		m_FileChanges.PushBack({ ChangeType::Removed, fileName });
	}
	void FileWatcher::onFileRenamedOld(const std::filesystem::path& fileName)
	{
		m_FileChanges.PushBack({ ChangeType::RenamedOld, fileName });
	}
	void FileWatcher::onFileRenamedNew(const std::filesystem::path& fileName)
	{
		m_FileChanges.PushBack({ ChangeType::RenamedNew, fileName });
	}
}