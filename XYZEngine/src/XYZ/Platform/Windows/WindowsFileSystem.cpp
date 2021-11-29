#include "stdafx.h"
#include "XYZ/Utils/FileSystem.h"

#include <filesystem>

#ifdef XYZ_PLATFORM_WINDOWS
#include <Windows.h>
#include <shlobj.h>
#include <commdlg.h>
#include <shellapi.h>

namespace XYZ {

	static bool s_IgnoreNextChange = false;

	static std::string wchar_to_string(wchar_t* input)
	{
		std::wstring string_input(input);
		std::string converted(string_input.begin(), string_input.end());
		return converted;
	}


	std::string FileSystem::OpenFile(void* windowHandle, const char* filter)
	{
		OPENFILENAMEA ofn;       // common dialog box structure
		CHAR szFile[MAX_PATH] = { 0 };       // if using TCHAR macros

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = (HWND)windowHandle;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;


		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return std::string();
	}

	static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
	{
		if (uMsg == BFFM_INITIALIZED)
		{
			std::string tmp = (const char*)lpData;
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
		}
		return 0;
	}

	std::string FileSystem::OpenFolder(void* windowHandle, const std::string& path)
	{
		TCHAR tmpPath[MAX_PATH];
		const std::wstring wsaved_path(path.begin(), path.end());
		const wchar_t* pathParam = wsaved_path.c_str();

		BROWSEINFO bi = { 0 };
		bi.lpszTitle = L"Browse for folder...";
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
		bi.lpfn = BrowseCallbackProc;
		bi.lParam = (LPARAM)pathParam;

		const LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

		if (pidl != 0)
		{
			//get the name of the folder and put it in path
			SHGetPathFromIDList(pidl, tmpPath);

			//free memory used
			IMalloc* imalloc = 0;
			if (SUCCEEDED(SHGetMalloc(&imalloc)))
			{
				imalloc->Free(pidl);
				imalloc->Release();
			}

			std::wstring tmp(tmpPath);
			std::string result(tmp.begin(), tmp.end());
			return result;
		}
		return "";
	}

	bool FileSystem::CreateFolder(const std::string& filepath)
	{
		const BOOL created = CreateDirectoryA(filepath.c_str(), NULL);
		if (!created)
		{
			const DWORD error = GetLastError();

			if (error == ERROR_ALREADY_EXISTS)
				XYZ_CORE_ERROR(filepath, " already exists!");

			if (error == ERROR_PATH_NOT_FOUND)
				XYZ_CORE_ERROR("One or more directories don't exist. ", filepath);

			return false;
		}

		return true;
	}

	bool FileSystem::Exists(const std::string& filepath)
	{
		const DWORD attribs = GetFileAttributesA(filepath.c_str());

		if (attribs == INVALID_FILE_ATTRIBUTES)
			return false;

		return true;
	}

	std::string FileSystem::Rename(const std::string& filepath, const std::string& newName)
	{
		s_IgnoreNextChange = true;
		const std::filesystem::path p = filepath;
		std::string newFilePath = p.parent_path().string() + "/" + newName + p.extension().string();
		MoveFileA(filepath.c_str(), newFilePath.c_str());
		s_IgnoreNextChange = false;
		return newFilePath;
	}

	bool FileSystem::MoveFileToPath(const std::string& filepath, const std::string& dest)
	{
		s_IgnoreNextChange = true;
		const std::filesystem::path p = filepath;
		const std::string destFilePath = dest + "/" + p.filename().string();
		const BOOL result = MoveFileA(filepath.c_str(), destFilePath.c_str());
		s_IgnoreNextChange = false;
		return result != 0;
	}

	bool FileSystem::DeleteFileAtPath(const std::string& filepath)
	{
		s_IgnoreNextChange = true;
		std::string fp = filepath;
		fp.append(1, '\0');
		SHFILEOPSTRUCTA file_op;
		file_op.hwnd = NULL;
		file_op.wFunc = FO_DELETE;
		file_op.pFrom = fp.c_str();
		file_op.pTo = "";
		file_op.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
		file_op.fAnyOperationsAborted = false;
		file_op.hNameMappings = 0;
		file_op.lpszProgressTitle = "";
		const int result = SHFileOperationA(&file_op);
		s_IgnoreNextChange = false;
		return result == 0;
	}
}

#endif