#pragma once
#include "FileWatcherListener.h"

#include "XYZ/Utils/Delegate.h"

namespace XYZ {

	class FileWatcher : public std::enable_shared_from_this<FileWatcher>
	{
	public:
		FileWatcher(const std::wstring& dir);
		~FileWatcher();

		template <auto Callable>
		void AddOnFileChange();

		template <auto Callable>
		void AddOnFileAdded();

		template <auto Callable>
		void AddOnFileRemoved();

		template <auto Callable>
		void AddOnFileRenamed();

		template <auto Callable, typename Type>
		void AddOnFileChange(Type* instance);

		template <auto Callable, typename Type>
		void AddOnFileAdded(Type* instance);

		template <auto Callable, typename Type>
		void AddOnFileRemoved(Type* instance);

		template <auto Callable, typename Type>
		void AddOnFileRenamed(Type* instance);





		template <auto Callable>
		void RemoveOnFileChange();

		template <auto Callable>
		void RemoveOnFileAdded();

		template <auto Callable>
		void RemoveOnFileRemoved();

		template <auto Callable>
		void RemoveOnFileRenamed();

		template <auto Callable, typename Type>
		void RemoveOnFileChange(Type* instance);

		template <auto Callable, typename Type>
		void RemoveOnFileAdded(Type* instance);

		template <auto Callable, typename Type>
		void RemoveOnFileRemoved(Type* instance);

		template <auto Callable, typename Type>
		void RemoveOnFileRenamed(Type* instance);

		void Start();
		void Stop();
		bool IsRunning();

		inline const std::wstring& GetDirectory() const { return m_Directory; }
	private:
		template <typename T>
		static void eraseFromVector(std::vector<T>& vec, const T& value);

		static void threadFunc(std::shared_ptr<FileWatcher> watcher);

		void onFileChange(const std::wstring& fileName);
		void onFileAdded(const std::wstring& fileName);
		void onFileRemoved(const std::wstring& fileName);
		void onFileRenamed(const std::wstring& fileName);

	private:
		std::wstring					  m_Directory;
		std::atomic<bool>				  m_Running;
		std::unique_ptr<std::thread>	  m_FileWatcherThread;
		std::mutex						  m_CallbacksMutex;

		std::vector<Delegate<void(const std::wstring& path)>> m_OnFileChange;
		std::vector<Delegate<void(const std::wstring& path)>> m_OnFileAdded;
		std::vector<Delegate<void(const std::wstring& path)>> m_OnFileRemoved;
		std::vector<Delegate<void(const std::wstring& path)>> m_OnFileRenamed;
	};
	template<auto Callable>
	inline void FileWatcher::AddOnFileChange()
	{
		std::scoped_lock lock(m_CallbacksMutex);
		Delegate<void(const std::wstring& path)> deleg;
		deleg.Connect<Callable>();
		m_OnFileChange.push_back(deleg);
	}
	template<auto Callable>
	inline void FileWatcher::AddOnFileAdded()
	{
		std::scoped_lock lock(m_CallbacksMutex);
		Delegate<void(const std::wstring& path)> deleg;
		deleg.Connect<Callable>();
		m_OnFileAdded.push_back(deleg);
	}
	template<auto Callable>
	inline void FileWatcher::AddOnFileRemoved()
	{
		std::scoped_lock lock(m_CallbacksMutex);
		Delegate<void(const std::wstring& path)> deleg;
		deleg.Connect<Callable>();
		m_OnFileRemoved.push_back(deleg);
	}
	template<auto Callable>
	inline void FileWatcher::AddOnFileRenamed()
	{
		std::scoped_lock lock(m_CallbacksMutex);
		Delegate<void(const std::wstring& path)> deleg;
		deleg.Connect<Callable>();
		m_OnFileRenamed.push_back(deleg);
	}

	template<auto Callable, typename Type>
	inline void FileWatcher::AddOnFileChange(Type*instance)
	{
		std::scoped_lock lock(m_CallbacksMutex);
		Delegate<void(const std::wstring& path)> deleg;
		deleg.Connect<Callable>(instance);
		m_OnFileChange.push_back(deleg);
	}
	template<auto Callable, typename Type>
	inline void FileWatcher::AddOnFileAdded(Type* instance)
	{
		std::scoped_lock lock(m_CallbacksMutex);
		Delegate<void(const std::wstring& path)> deleg;
		deleg.Connect<Callable>(instance);
		m_OnFileAdded.push_back(deleg);
	}
	template<auto Callable, typename Type>
	inline void FileWatcher::AddOnFileRemoved(Type* instance)
	{
		std::scoped_lock lock(m_CallbacksMutex);
		Delegate<void(const std::wstring& path)> deleg;
		deleg.Connect<Callable>(instance);
		m_OnFileRemoved.push_back(deleg);
	}
	template<auto Callable, typename Type>
	inline void FileWatcher::AddOnFileRenamed(Type* instance)
	{
		std::scoped_lock lock(m_CallbacksMutex);
		Delegate<void(const std::wstring& path)> deleg;
		deleg.Connect<Callable>(instance);
		m_OnFileRenamed.push_back(deleg);
	}
	template<auto Callable>
	inline void FileWatcher::RemoveOnFileChange()
	{
		std::scoped_lock lock(m_CallbacksMutex);
		Delegate<void(const std::wstring& path)> deleg;
		deleg.Connect<Callable>();
		eraseFromVector(m_OnFileChange, deleg);
	}
	template<auto Callable>
	inline void FileWatcher::RemoveOnFileAdded()
	{
		std::scoped_lock lock(m_CallbacksMutex);
		Delegate<void(const std::wstring& path)> deleg;
		deleg.Connect<Callable>();
		eraseFromVector(m_OnFileAdded, deleg);
	}
	template<auto Callable>
	inline void FileWatcher::RemoveOnFileRemoved()
	{
		std::scoped_lock lock(m_CallbacksMutex);
		Delegate<void(const std::wstring& path)> deleg;
		deleg.Connect<Callable>();
		eraseFromVector(m_OnFileRemoved, deleg);
	}
	template<auto Callable>
	inline void FileWatcher::RemoveOnFileRenamed()
	{
		std::scoped_lock lock(m_CallbacksMutex);
		Delegate<void(const std::wstring& path)> deleg;
		deleg.Connect<Callable>();
		eraseFromVector(m_OnFileRenamed, deleg);
	}
	template<auto Callable, typename Type>
	inline void FileWatcher::RemoveOnFileChange(Type* instance)
	{
		std::scoped_lock lock(m_CallbacksMutex);
		Delegate<void(const std::wstring& path)> deleg;
		deleg.Connect<Callable>(instance);
		eraseFromVector(m_OnFileChange, deleg);
	}
	template<auto Callable, typename Type>
	inline void FileWatcher::RemoveOnFileAdded(Type* instance)
	{
		std::scoped_lock lock(m_CallbacksMutex);
		Delegate<void(const std::wstring& path)> deleg;
		deleg.Connect<Callable>(instance);
		eraseFromVector(m_OnFileAdded, deleg);
	}
	template<auto Callable, typename Type>
	inline void FileWatcher::RemoveOnFileRemoved(Type* instance)
	{
		std::scoped_lock lock(m_CallbacksMutex);
		Delegate<void(const std::wstring& path)> deleg;
		deleg.Connect<Callable>(instance);
		eraseFromVector(m_OnFileRemoved, deleg);
	}
	template<auto Callable, typename Type>
	inline void FileWatcher::RemoveOnFileRenamed(Type* instance)
	{
		std::scoped_lock lock(m_CallbacksMutex);
		Delegate<void(const std::wstring& path)> deleg;
		deleg.Connect<Callable>(instance);
		eraseFromVector(m_OnFileRenamed, deleg);
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