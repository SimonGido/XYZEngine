#pragma once

#include "XYZ/Core/Timestep.h"
#include "XYZ/FileWatcher/FileWatcher.h"

#include <mutex>

struct lua_State;
namespace XYZ {

	class LuaApp : public IFileWatcherListener
	{
	public:
		LuaApp(const std::string& directory, const std::string& filename);
		~LuaApp();

		void OnUpdate(Timestep ts);
	

		virtual void OnFileChange(const std::wstring& filepath) override;
		virtual void OnFileAdded(const std::wstring& filepath) override;
		virtual void OnFileRemoved(const std::wstring& filepath) override;
		virtual void OnFileRenamed(const std::wstring& filepath) override;

	private:
		bool tryReload();

	private:
		lua_State* m_L;
		std::unique_ptr<FileWatcher> m_FileWatcher;

		std::string m_Directory;
		std::string m_FileName;

		std::mutex m_Mutex;

		bool m_Reload = false;
	};
}