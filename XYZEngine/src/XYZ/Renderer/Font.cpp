#include "stdafx.h"
#include "Font.h"

#include "XYZ/Utils/DataStructures/ByteBuffer.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stbi_image_write.h>

namespace XYZ {

	Font::Font(uint32_t pixelSize, const std::string& path)
		:
		m_PixelSize(pixelSize)
	{
		FT_Library ft;
		XYZ_ASSERT(!FT_Init_FreeType(&ft), "Could not init FreeType library");

		FT_Face face;
		XYZ_ASSERT(!FT_New_Face(ft, path.c_str(), 0, &face), "Failed to load font ", path);

		XYZ_ASSERT(!FT_Set_Pixel_Sizes(face, 0, pixelSize), "Unsupported pixel size")
			m_Characters.resize(sc_NumGlyphs);

		// quick and dirty max texture size estimate
		uint32_t maxDim = (1 + (face->size->metrics.height >> 6)) * ceilf(sqrtf(sc_NumGlyphs));
		uint32_t texWidth = 1;
		while (texWidth < maxDim)
			texWidth <<= 1;
		uint32_t texHeight = texWidth;

		ByteBuffer pixelData;
		pixelData.Allocate(texWidth * texHeight * 1);
		pixelData.ZeroInitialize();

		uint32_t penX = 0, penY = 0;
		for (uint8_t c = 0; c < sc_NumGlyphs; c++)
		{
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				XYZ_LOG_ERR("Failed to load char ", c);
				return;
			}
			FT_Bitmap* bmp = &face->glyph->bitmap;
			if (penX + bmp->width >= texWidth)
			{
				penX = 0;
				penY += ((face->size->metrics.height >> 6) + 1);
			}

			for (uint32_t row = 0; row < bmp->rows; ++row)
			{
				for (uint32_t col = 0; col < bmp->width; ++col)
				{
					uint32_t x = penX + col;
					uint32_t y = penY + row;
					pixelData[y * texWidth + x] = bmp->buffer[row * bmp->pitch + col];
				}
			}
	
			m_Characters[c].X0Coord = penX;
			m_Characters[c].Y0Coord = penY;
			m_Characters[c].X1Coord = penX + bmp->width;
			m_Characters[c].Y1Coord = penY + bmp->rows;
			m_Characters[c].XOffset = face->glyph->bitmap_left;
			m_Characters[c].YOffset = face->glyph->bitmap_top;
			m_Characters[c].XAdvance = face->glyph->advance.x >> 6;

			penX += bmp->width + 1;
		}

		// Alternative for space TODO: fix this
		uint8_t dash = (uint8_t)'_';
		uint8_t space = (uint8_t)' ';

		m_Characters[space] = m_Characters[dash];

		ByteBuffer pngData;
		pngData.Allocate(texWidth * texHeight * 4);
		for (int i = 0; i < (texWidth * texHeight); ++i)
		{
			pngData[i * 4 + 0] |= pixelData[i];
			pngData[i * 4 + 1] |= pixelData[i];
			pngData[i * 4 + 2] |= pixelData[i];
			pngData[i * 4 + 3] = pixelData[i];
		}

		m_Texture = Texture2D::Create({
			texWidth,texHeight,4,
			TextureWrap::Clamp,
			TextureFormat::RGB,
			TextureParam::Linear,
			TextureParam::Linear
			});

		m_Texture->SetData(pngData, pngData.GetSize());
		stbi_write_png("font_output.png", texWidth, texHeight, 4, pngData, texWidth * 4);

		FT_Done_Face(face);
		FT_Done_FreeType(ft);

		delete[]pixelData;
		delete[]pngData;
	}
}