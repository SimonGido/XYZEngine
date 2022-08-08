#pragma once

namespace XYZ::Utils {
	std::string& ToLower(std::string& string);
	std::string ToLowerCopy(const std::string& string);
	std::string GetFilename(const std::string& filepath);
	std::string GetFilenameWithoutExtension(const std::string& filepath);
	std::string GetDirectoryPath(const std::string& filepath);
	std::string GetExtension(const std::string& filename);
	std::string RemoveExtension(const std::string& filename);
	bool StartsWith(const std::string& string, const std::string& start);
	std::vector<std::string> SplitString(const std::string& string, const std::string& delimiters);
	std::vector<std::string> SplitString(const std::string& string, const char delimiter);
	std::string FirstSubString(const std::string& string, const char delimiter);
	std::string_view FirstSubString(const std::string_view string, const char delimiter);
	std::string BytesToString(uint64_t bytes);
	const char* FindToken(const char* str, const std::string& token);
}