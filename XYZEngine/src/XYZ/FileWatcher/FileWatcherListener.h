#pragma once
#include "XYZ/Core/Core.h"

#include <string>

namespace XYZ {
	class XYZ_API FileWatcherListener
	{
	public:
		virtual ~FileWatcherListener() = default;

	protected:
		virtual void OnFileChange(const std::wstring& path) = 0;
		virtual void OnFileAdded(const std::wstring& path) = 0;
		virtual void OnFileRemoved(const std::wstring& path) = 0;
		virtual void OnFileRenamed(const std::wstring& path) = 0;

		friend class FileWatcher;
	};
}