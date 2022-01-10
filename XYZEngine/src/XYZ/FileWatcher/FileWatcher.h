#pragma once
#include "FileWatcherListener.h"

namespace XYZ {

	class FileWatcher : public std::enable_shared_from_this<FileWatcher>
	{
	public:
		FileWatcher(const std::wstring& dir);
		~FileWatcher();

		template <typename T, typename ...Args>
		void AddListener(Args&& ...args);

		void Start();
		void Stop();
		bool IsRunning();

		inline const std::wstring& GetDirectory() const { return m_Directory; }
	private:
		static void threadFunc(std::shared_ptr<FileWatcher> watcher);

		void onFileChange(const std::wstring& fileName);
		void onFileAdded(const std::wstring& fileName);
		void onFileRemoved(const std::wstring& fileName);
		void onFileRenamed(const std::wstring& fileName);

	private:
		std::wstring					  m_Directory;
		std::atomic<bool>				  m_Running;
		std::unique_ptr<std::thread>	  m_FileWatcherThread;
		std::vector<FileWatcherListener*> m_Listeners;
	};

	template<typename T, typename ...Args>
	inline void FileWatcher::AddListener(Args&& ...args)
	{
		if (!m_Running)
		{
			m_Listeners.push_back(new T(std::forward<Args>(args)...));
		}
		else
		{
			XYZ_WARN("Can not add listener while running");
		}
	}
}