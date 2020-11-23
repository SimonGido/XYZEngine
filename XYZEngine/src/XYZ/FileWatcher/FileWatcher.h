#pragma once

#include "FileWatcherListener.h"


namespace XYZ {

	class FileWatcher
	{
	public:
		FileWatcher(const std::wstring& dir);
		void AddListener(IFileWatcherListener* listener);

		virtual void Start() = 0;
		virtual void Stop() = 0;
		virtual bool IsRunning() const = 0;

		inline const std::wstring& GetDirectory() const { return m_Directory; }

		void OnFileChange(const std::wstring& fileName);
		void OnFileAdded(const std::wstring& fileName);
		void OnFileRemoved(const std::wstring& fileName);
		void OnFileRenamed(const std::wstring& fileName);


		static std::unique_ptr<FileWatcher> Create(const std::wstring& dir);
	protected:
		std::wstring m_Directory;
		std::vector<IFileWatcherListener*> m_Listeners;
	};
}