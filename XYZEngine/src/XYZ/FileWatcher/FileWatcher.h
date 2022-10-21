#pragma once
#include "FileWatcherListener.h"

#include "XYZ/Utils/Delegate.h"
#include "XYZ/Utils/DataStructures/ThreadQueue.h"

#include <filesystem>

namespace XYZ {

	class FileWatcher : public std::enable_shared_from_this<FileWatcher>
	{
	public:
		enum class ChangeType { Modified, Added, Removed, RenamedOld, RenamedNew };

		FileWatcher(const std::filesystem::path& dir);
		~FileWatcher();

		template <auto Callable>
		void AddOnFileChanged();

		template <auto Callable, typename Type>
		void AddOnFileChanged(Type* instance);


		template <auto Callable>
		void RemoveOnFileChanged();

		template <auto Callable, typename Type>
		void RemoveOnFileChanged(Type* instance);


		void Start();
		void Stop();
		bool IsRunning();

		void ProcessChanges();

		inline const std::filesystem::path& GetDirectory() const { return m_Directory; }
	private:
		template <typename T>
		static void eraseFromVector(std::vector<T>& vec, const T& value);

		static void threadFunc(std::shared_ptr<FileWatcher> watcher);

		void onFileModified(  const std::filesystem::path& fileName);
		void onFileAdded(     const std::filesystem::path& fileName);
		void onFileRemoved(   const std::filesystem::path& fileName);
		void onFileRenamedOld(const std::filesystem::path& fileName);
		void onFileRenamedNew(const std::filesystem::path& fileName);

	private:
		std::filesystem::path			  m_Directory;
		std::atomic<bool>				  m_Running;
		std::unique_ptr<std::thread>	  m_FileWatcherThread;

		std::vector<Delegate<void(ChangeType, const std::filesystem::path&)>> m_OnFileChange;
	
		struct Change
		{
			ChangeType			  Type;
			std::filesystem::path FilePath;
		};
		ThreadQueue<Change> m_FileChanges;
	};

	template<auto Callable>
	inline void FileWatcher::AddOnFileChanged()
	{
		Delegate<void(ChangeType, const std::filesystem::path&)> deleg;
		deleg.Connect<Callable>();
		m_OnFileChange.push_back(deleg);
	}
	template<auto Callable, typename Type>
	inline void FileWatcher::AddOnFileChanged(Type* instance)
	{
		Delegate<void(ChangeType, const std::filesystem::path&)> deleg;
		deleg.Connect<Callable>(instance);
		m_OnFileChange.push_back(deleg);
	}
	template<auto Callable>
	inline void FileWatcher::RemoveOnFileChanged()
	{
		Delegate<void(ChangeType, const std::filesystem::path&)> deleg;
		deleg.Connect<Callable>();
		eraseFromVector(m_OnFileChange, deleg);
	}
	template<auto Callable, typename Type>
	inline void FileWatcher::RemoveOnFileChanged(Type* instance)
	{
		Delegate<void(ChangeType, const std::filesystem::path&)> deleg;
		deleg.Connect<Callable>(instance);
		eraseFromVector(m_OnFileChange, deleg);
	}
	template<typename T>
	inline void FileWatcher::eraseFromVector(std::vector<T>& vec, const T& value)
	{
		for (auto it = vec.begin(); it != vec.end();)
		{
			if ((*it) == value)
			{
				it = vec.erase(it);
			}
			else
			{
				it++;
			}
		}
	}
}