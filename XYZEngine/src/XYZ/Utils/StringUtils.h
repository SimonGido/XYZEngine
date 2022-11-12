#pragma once

#include "XYZ/Core/Core.h"

namespace XYZ::Utils {
	XYZ_API std::string& ToLower(std::string& string);
	XYZ_API std::string ToLowerCopy(const std::string& string);
	XYZ_API std::string GetFilename(const std::string& filepath);
	XYZ_API std::string GetFilenameWithoutExtension(const std::string& filepath);
	XYZ_API std::string GetDirectoryPath(const std::string& filepath);
	XYZ_API std::string GetExtension(const std::string& filename);
	XYZ_API std::string RemoveExtension(const std::string& filename);
	XYZ_API bool StartsWith(const std::string& string, const std::string& start);
	XYZ_API std::vector<std::string> SplitString(const std::string& string, const std::string& delimiters);
	XYZ_API std::vector<std::string> SplitString(const std::string& string, const char delimiter);
	XYZ_API std::string FirstSubString(const std::string& string, const char delimiter);
	XYZ_API std::string_view FirstSubString(const std::string_view string, const char delimiter);
	XYZ_API std::string BytesToString(uint64_t bytes);
	XYZ_API const char* FindToken(const char* str, const std::string& token);
}