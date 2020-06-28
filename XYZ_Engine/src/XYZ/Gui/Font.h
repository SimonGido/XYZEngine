#pragma once

#include <unordered_map>

namespace XYZ {
	struct Character
	{
		uint32_t XCoord;
		uint32_t YCoord;
		uint32_t Width;
		uint32_t Height;
		uint32_t XAdvance;
		int32_t XOffset;
		int32_t YOffset;
	};

	struct FontData
	{
		std::string Name;
		uint32_t ScaleW;
		uint32_t ScaleH;
		uint32_t LineHeight;
		uint32_t NumCharacters;
		uint32_t PaddingWidth;
		uint32_t PaddingHeight;
	};

	class Font
	{
	public:
		Font(const std::string& configPath);

		const FontData& GetData() const { return m_Data; }
		const Character& GetCharacter(uint8_t ch) const;
	private:
		void parseHeaderInfo(const std::string& source);
		void parseCharacters(const std::string& source);

		size_t endOfToken(const char* token, const std::string& source, size_t pos);
		std::string readFile(const std::string& filepath);

	private:
		FontData m_Data;
		std::unordered_map<uint8_t, Character> m_Characters;
	};
}