#pragma once
#include "XYZ/FileWatcher/FileWatcher.h"

#include <thread>

namespace XYZ {

	class WindowsFileWatcher : public FileWatcher
	{
	public:
		WindowsFileWatcher(const std::wstring& dir);

		virtual void Start() override;
		virtual void Stop() override;
		virtual bool IsRunning() const override { return m_Running; }

	private:
		std::unique_ptr<std::thread> m_FileWatcherThread;
		std::atomic<bool>			 m_Running;
	};
}