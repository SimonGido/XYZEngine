#pragma once

#include "XYZ/Core/Timestep.h"
#include "XYZ/FileWatcher/FileWatcher.h"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>


#include <mutex>

namespace XYZ {

	class LuaModule : public IFileWatcherListener
	{
	public:
		LuaModule(const std::string& directory, const std::string& filename);
		~LuaModule();

		void OnUpdate(Timestep ts);
	

	protected:
		virtual void OnFileChange(const std::wstring& filepath) override;
		virtual void OnFileAdded(const std::wstring& filepath) override;
		virtual void OnFileRemoved(const std::wstring& filepath) override;
		virtual void OnFileRenamed(const std::wstring& filepath) override;

	private:
		bool tryReload();

	private:
		sol::state m_L;
		std::unique_ptr<FileWatcher> m_FileWatcher;

		std::string m_Directory;
		std::string m_FileName;

		std::mutex m_Mutex;

		bool m_Reload = false;
	};
}