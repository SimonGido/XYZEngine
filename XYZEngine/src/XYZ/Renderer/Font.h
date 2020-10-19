#pragma once
#include "XYZ/Core/Ref.h"
#include "XYZ/Renderer/Texture.h"



namespace XYZ {

	struct Character
	{
		uint32_t X0Coord;
		uint32_t Y0Coord;
		uint32_t X1Coord;
		uint32_t Y1Coord;
		uint32_t XOffset;
		uint32_t YOffset;
		uint32_t XAdvance;
	};

	class Font : public RefCount
	{
	public:
		Font(uint32_t pixelSize, const std::string& path);

		const Ref<Texture2D>& GetTexture() const { return m_Texture; }
		const Character& GetCharacter(uint8_t index) const { return m_Characters[index]; }
		const uint32_t GetWidth() const { return m_Texture->GetWidth(); }
		const uint32_t GetHeight() const { return m_Texture->GetHeight(); }	
	private:
		uint32_t m_PixelSize;
		Ref<Texture2D> m_Texture;
		std::vector<Character> m_Characters;

		static constexpr uint32_t sc_NumGlyphs = 128;
	};
	
}