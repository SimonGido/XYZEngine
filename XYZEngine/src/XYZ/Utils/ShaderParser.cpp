#include "stdafx.h"
#include "ShaderParser.h"

namespace XYZ {

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


	ShaderParser::ShaderParser()
	{

	}

	void ShaderParser::AddKeyword(std::string keyword)
	{
		m_Keywords.push_back(std::move(keyword));
	}
	void ShaderParser::RemoveKeywordsFromSourceCode(std::string& source)
	{
		for (auto& word : m_Keywords)
		{
			auto pos = source.find(word);
			while (pos != std::string::npos)
			{
				source.erase(pos, word.length());
				pos = source.find(word);
			}
		}
	}


	std::unordered_map<std::string, std::string> ShaderParser::ParseStages(const std::string& source) const
	{
		std::unordered_map<std::string, std::string> stages;
		extractSources(source, stages);
		return stages;
	}

	std::unordered_map<std::string, ShaderParser::ShaderLayoutInfo> ShaderParser::ParseLayoutInfo(const std::string& source) const
	{
		std::unordered_map<std::string, ShaderParser::ShaderLayoutInfo> result;

		size_t offset = 0;
		ShaderLayoutInfo info;

		while (parseLayout(source, offset, info, offset))
		{
			result[info.Name] = info;
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

	void ShaderParser::extractSources(const std::string& source, std::unordered_map<std::string, std::string>& stages) const
	{
		const char* typeToken = "#type";
		const size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos)
		{
			const size_t eol = source.find_first_of("\r\n", pos);
			XYZ_ASSERT(eol != std::string::npos, "Syntax error");
			const size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);

			const size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);
			stages[type] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
		}
	}
}