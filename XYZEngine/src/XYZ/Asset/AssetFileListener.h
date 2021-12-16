#pragma once
#include "XYZ/FileWatcher/FileWatcherListener.h"

namespace XYZ {

	class AssetFileListener : public FileWatcherListener
	{
	public:
		virtual void OnFileChange(const std::wstring& path) override;
		virtual void OnFileAdded(const std::wstring& path) override;
		virtual void OnFileRemoved(const std::wstring& path) override;
		virtual void OnFileRenamed(const std::wstring& path) override;

	};
}