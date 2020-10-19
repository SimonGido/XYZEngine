#include "stdafx.h"
#include "Serializable.h"

namespace XYZ {
	static std::string ExtractNameFromFilePath(const std::string filepath)
	{
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		return filepath.substr(lastSlash, count);
	}


	Serializable::Serializable(const std::string& filepath)
		:
		m_Filepath(filepath),
		m_Name(ExtractNameFromFilePath(filepath))
	{
	}

	void Serializable::SetFilepath(const std::string& filepath)
	{
		m_Filepath = filepath;
		m_Name = ExtractNameFromFilePath(filepath);
	}
}