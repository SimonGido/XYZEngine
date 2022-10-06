#include "stdafx.h"
#include "ShaderParser.h"

namespace XYZ {

	static ShaderParser::ShaderStage ShaderStageFromString(const std::string& type)
	{
		if (type == "vertex")
			return ShaderParser::ShaderStage::Vertex;
		if (type == "fragment" || type == "pixel")
			return ShaderParser::ShaderStage::Fragment;
		if (type == "compute")
			return ShaderParser::ShaderStage::Compute;

		XYZ_ASSERT(false, "Unknown shader type!");
		return ShaderParser::ShaderStage::None;
	}

	static size_t FindDigit(const std::string& str, size_t offset = 0)
	{
		while (offset != std::string::npos)
		{
			if (std::isdigit(str[offset]))
				return offset;
			offset++;
		}
	}

	static std::pair<size_t, size_t> ExtractNumber(const std::string& str, size_t offset = 0)
	{
		XYZ_ASSERT(std::isdigit(str[offset]), "");

		size_t endOfNumber = offset;
		while (endOfNumber != std::string::npos && std::isdigit(str[endOfNumber]))
		{
			endOfNumber++;
		}
		return { offset, endOfNumber };
	}


	static long StringToNumber(const std::string& str, size_t start, size_t end)
	{
		XYZ_ASSERT(str.size() > start && str.size() > end, "");

		char* endPointer = (char*)&str.c_str()[end];

		return std::strtol(&str.c_str()[start], &endPointer, 10);
	}


	ShaderParser::ShaderParser(std::string sourceCode)
		:
		m_SourceCode(std::move(sourceCode))
	{
		extractSources();
	}
	void ShaderParser::AddKeyword(std::string keyword)
	{
		m_Keywords.push_back(std::move(keyword));
	}
	void ShaderParser::RemoveKeywordsFromSourceCode()
	{
		for (auto& word : m_Keywords)
		{
			auto pos = m_SourceCode.find(word);
			while (pos != std::string::npos)
			{
				m_SourceCode.erase(pos, word.length());
				pos = m_SourceCode.find(word);
			}
		}
	}

	std::vector<ShaderParser::ShaderLayoutInfo> ShaderParser::ParseLayoutInfo(ShaderStage stage) const
	{
		std::vector<ShaderLayoutInfo> result;

		size_t offset = 0;
		ShaderLayoutInfo info;

		auto it = m_Sources.find(stage);
		if (it != m_Sources.end())
		{
			while (parseLayout(it->second, offset, info, offset))
			{
				result.push_back(info);
			}
		}
		return result;
	}
	size_t ShaderParser::findKeywordInSourceCode(const std::string& source, size_t start, size_t end, std::string& foundWord) const
	{
		std::string_view view(&source.c_str()[start], end - start);
		for (auto& word : m_Keywords)
		{
			size_t result = view.find(word);
			if (result != std::string::npos)
			{
				foundWord = word;
				return result;
			}
		}
		return std::string::npos;
	}
	bool ShaderParser::parseLayout(const std::string& source, size_t offset, ShaderLayoutInfo& info, size_t& end) const
	{
		size_t layoutPos = source.find("layout", offset);
		info.Keyword.clear();
		info.Name.clear();

		if (layoutPos != std::string::npos)
		{
			const size_t locationPos = source.find("location", layoutPos);
			if (locationPos != std::string::npos)
			{
				const size_t locationStartDigit = FindDigit(source, locationPos);

				auto [startNumberPosition, endNumberPosition] = ExtractNumber(source, locationStartDigit);
				const size_t locationEndBracketPos = source.find(')', endNumberPosition);
				const size_t layoutEndPosition = source.find(';', locationEndBracketPos);

				const uint32_t location = StringToNumber(source, startNumberPosition, endNumberPosition);

				const size_t keywordPosition = findKeywordInSourceCode(source, offset, locationPos, info.Keyword);

				size_t endOfName = layoutEndPosition - 1;
				while (source[endOfName] == ' '
					|| source[endOfName] == '\t')
				{
					endOfName--;
				}
				size_t startOfName = endOfName;
				while (source[startOfName] != ' '
					&& source[startOfName] != '\t')
				{
					startOfName--;
				}

				info.Name = source.substr(startOfName + 1, endOfName - startOfName);
				info.Location = location;

				end = layoutEndPosition;
				return true;
			}
		}
		return false;
	}
	void ShaderParser::extractSources()
	{
		const char* TypeToken = "#type";
		const size_t TypeTokenLength = strlen(TypeToken);
		size_t pos = m_SourceCode.find(TypeToken, 0);
		while (pos != std::string::npos)
		{
			const size_t eol = m_SourceCode.find_first_of("\r\n", pos);
			XYZ_ASSERT(eol != std::string::npos, "Syntax error");
			const size_t begin = pos + TypeTokenLength + 1;
			std::string type = m_SourceCode.substr(begin, eol - begin);

			ShaderParser::ShaderStage stage = ShaderStageFromString(type);

			const size_t nextLinePos = m_SourceCode.find_first_not_of("\r\n", eol);
			pos = m_SourceCode.find(TypeToken, nextLinePos);
			m_Sources[stage] = m_SourceCode.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? m_SourceCode.size() - 1 : nextLinePos));
		}
	}
}