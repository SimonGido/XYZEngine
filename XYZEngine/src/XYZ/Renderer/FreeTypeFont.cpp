#include "stdafx.h"
#include "FreeTypeFont.h"


#include "XYZ/Utils/DataStructures/ByteBuffer.h"

#include <ft2build.h>
#include FT_FREETYPE_H  


namespace XYZ {
	FreeTypeFont::FreeTypeFont(uint32_t width, uint32_t height, const std::string& path)
	{
		FT_Library ft;
		XYZ_ASSERT(!FT_Init_FreeType(&ft), "Could not init FreeType library");

	
		FT_Face face;
		XYZ_ASSERT(!FT_New_Face(ft, path.c_str(), 0, &face), "Failed to load font ", path);

		XYZ_ASSERT(!FT_Set_Pixel_Sizes(face, 0, 16), "Unsupported pixel size")
		ByteBuffer buffer;	
		buffer.Allocate(100000);
		m_Characters.resize(128);
		uint32_t offset = 0;
		uint32_t textureWidth = 0;
		uint32_t textureHeight = 0;
		for (uint8_t c = 0; c < 128; c++)
		{
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				XYZ_LOG_ERR("Failed to load glyph ", c);
				continue;
			}
			uint32_t numPixels = face->glyph->bitmap.width * face->glyph->bitmap.rows;
			
			
			buffer.Write(face->glyph->bitmap.buffer, numPixels * sizeof(uint8_t), offset);
			offset += numPixels * sizeof(uint8_t);
			
			textureWidth += face->glyph->bitmap.width;
			textureHeight = std::max(textureHeight, face->glyph->bitmap.rows);

			FreeTypeCharacter character = {
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				face->glyph->bitmap_left,
				face->glyph->bitmap_top,
				face->glyph->advance.x,
			};
			m_Characters[c] = character;
		}
		
		m_Texture = Texture2D::Create({
			textureWidth,textureHeight,
			1,
			TextureWrap::Clamp, TextureFormat::RGB,
			TextureParam::Linear, TextureParam::Linear 
			});
		m_Texture->SetData(buffer, textureWidth * textureHeight * 1);

		FT_Done_Face(face);
		FT_Done_FreeType(ft);
		delete[]buffer;
	}
}