#include "stdafx.h"	
#include "AssetFileListener.h"

#include "AssetManager.h"

namespace XYZ {
	void AssetFileListener::OnFileChange(const std::wstring& path)
	{
		std::string strPath(path.begin(), path.end());
		std::replace(strPath.begin(), strPath.end(), '\\', '/');
		if (strPath.find(".meta") == std::string::npos)
			AssetManager::ReloadAsset("Assets/" + strPath + ".meta");
	}
	void AssetFileListener::OnFileAdded(const std::wstring& path)
	{
		std::string strPath(path.begin(), path.end());
	}
	void AssetFileListener::OnFileRemoved(const std::wstring& path)
	{
		std::string strPath(path.begin(), path.end());
	}
	void AssetFileListener::OnFileRenamed(const std::wstring& path)
	{
		std::string strPath(path.begin(), path.end());
	}
}