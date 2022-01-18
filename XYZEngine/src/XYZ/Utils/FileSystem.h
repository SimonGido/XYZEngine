#pragma once

namespace XYZ {
	class FileSystem
	{
	public:
		static std::string OpenFile(void* windowHandle, const char* filter = "All\0*.*\0");
		static std::string OpenFolder(void* windowHandle, const std::string& path);

		static bool CreateFolder(const std::string& filepath);
		static bool Exists(const std::string& filepath);
		static std::string Rename(const std::string& filepath, const std::string& newName);
		static bool DeleteFileAtPath(const std::string& filepath);
		static bool MoveFileToPath(const std::string& filepath, const std::string& dest);

		static std::string UniqueFilePath(const std::string& dir, const std::string& fileName, const char* extension);
	};
}