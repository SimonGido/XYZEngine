#include "stdafx.h"
#include "Font.h"


namespace XYZ {
	Font::Font(const std::string& configPath)
	{
		std::string source = readFile(configPath);
		parseHeaderInfo(source);
		parseCharacters(source);
	}

	void Font::parseHeaderInfo(const std::string& source)
	{
		{
			size_t pos = endOfToken("face=\"", source, 0);
			size_t endWord = source.find_first_of("\"", pos);
			m_Data.Name = source.substr(pos, endWord - pos);
		}
		{
			size_t pos = endOfToken("padding=", source, 0);
			size_t endWord = source.find_first_of(",", pos);
			m_Data.PaddingWidth = static_cast<uint32_t>(std::stoul((source.substr(pos, endWord - pos))));
			m_Data.PaddingHeight = static_cast<uint32_t>(std::stoul((source.substr(pos+2, endWord - pos))));
		}
		{
			size_t pos = endOfToken("lineHeight=", source, 0);
			size_t endWord = source.find_first_of(" ", pos);
			m_Data.LineHeight = static_cast<uint32_t>(std::stoul((source.substr(pos, endWord - pos))));
		}
		{
			size_t pos = endOfToken("scaleW=", source, 0);
			size_t endWord = source.find_first_of(" ", pos);
			m_Data.ScaleW = static_cast<uint32_t>(std::stoul((source.substr(pos, endWord - pos))));
		}
		{
			size_t pos = endOfToken("scaleH=", source, 0);
			size_t endWord = source.find_first_of(" ", pos);
			m_Data.ScaleH = static_cast<uint32_t>(std::stoul((source.substr(pos, endWord - pos))));
		}
		{
			size_t pos = endOfToken("chars count=", source, 0);
			size_t endWord = source.find_first_of(" ", pos);
			m_Data.NumCharacters = static_cast<uint32_t>(std::stoul((source.substr(pos, endWord - pos))));
		}

	}

	void Font::parseCharacters(const std::string& source)
	{
		size_t pos = 0;
		uint32_t counter = 0;
		while (pos != std::string::npos && counter != m_Data.NumCharacters)
		{
			// ID
			uint8_t id = 0;
			Character character;
			{
				size_t eofToken = endOfToken("char id=", source, pos);
				size_t endWord = source.find_first_of(" ", eofToken);
				id = static_cast<uint8_t>(std::stoul((source.substr(eofToken, endWord - eofToken))));
				pos = endWord;
			}
			// X
			{
				size_t eofToken = endOfToken("x=", source, pos);
				size_t endWord = source.find_first_of(" ", eofToken);	 
				character.XCoord = static_cast<uint32_t>(std::stoul((source.substr(eofToken, endWord - eofToken))));
				pos = endWord;
			}
			// Y
			{
				size_t eofToken = endOfToken("y=", source, pos);
				size_t endWord = source.find_first_of(" ", eofToken);
				character.YCoord = static_cast<uint32_t>(std::stoul((source.substr(eofToken, endWord - eofToken))));
				pos = endWord;
			}
			// Width
			{
				size_t eofToken = endOfToken("width=", source, pos);
				size_t endWord = source.find_first_of(" ", eofToken);
				character.Width = static_cast<uint32_t>(std::stoul((source.substr(eofToken, endWord - eofToken))));
				pos = endWord;
			}
			// Height
			{
				size_t eofToken = endOfToken("height=", source, pos);
				size_t endWord = source.find_first_of(" ", eofToken);
				character.Height = static_cast<uint32_t>(std::stoul((source.substr(eofToken, endWord - eofToken))));
				pos = endWord;
			}
			// XOffset
			{
				size_t eofToken = endOfToken("xoffset=", source, pos);
				size_t endWord = source.find_first_of(" ", eofToken);
				character.XOffset = static_cast<int32_t>(std::stoi((source.substr(eofToken, endWord - eofToken))));
				pos = endWord;
			}
			// YOffset
			{
				size_t eofToken = endOfToken("yoffset=", source, pos);
				size_t endWord = source.find_first_of(" ", eofToken);
				character.YOffset = static_cast<int32_t>(std::stoi((source.substr(eofToken, endWord - eofToken))));
				pos = endWord;
			}

			// XAdvance
			{
				size_t eofToken = endOfToken("xadvance=", source, pos);
				size_t endWord = source.find_first_of(" ", eofToken);
				character.XAdvance = static_cast<uint32_t>(std::stoul((source.substr(eofToken, endWord - eofToken))));
				pos = endWord;
			}

			m_Characters[id] = character;
			counter++;
		}

	}

	size_t Font::endOfToken(const char* token, const std::string& source, size_t pos)
	{
		return source.find(token, pos) + strlen(token);
	}

	std::string Font::readFile(const std::string& filepath)
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
		}
		else
			XYZ_LOG_ERR("Could not open file ", filepath.c_str());


		return result;
	}


	const Character& Font::GetCharacter(uint8_t ch) const
	{
		XYZ_ASSERT(ch < sc_NumCharacters, "Character index out of range");
		return m_Characters[ch];
	}
	
}