#pragma once
#include "XYZ/Renderer/Texture.h"


namespace XYZ {

	struct FreeTypeCharacter
	{
		uint32_t Width;
		uint32_t Height;
		uint32_t XCoord;
		uint32_t YCoord;
		uint32_t XAdvance;
	};

	class FreeTypeFont
	{
	public:
		FreeTypeFont(uint32_t width, uint32_t height, const std::string& path);

		const Ref<Texture2D>& GetTexture() const { return m_Texture; }

	private:
		Ref<Texture2D> m_Texture;
		std::vector<FreeTypeCharacter> m_Characters;
	};
}