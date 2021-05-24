#pragma once
#include "InGuiWindow.h"

namespace XYZ {
	struct InGuiFrame
	{
		InGuiFrame();
		~InGuiFrame();

		uint32_t CustomTextureID() const { return (uint32_t)CustomTextures.size() + InGuiConfig::sc_FontTexture; }
		
		InGuiWindow*			   CurrentWindow;
		glm::vec2				   MovedWindowOffset;
		float					   CurrentMenuWidth;
		bool					   HighlightNext;
		uint32_t				   CurrentTreeDepth;
		std::vector<InGuiID>	   MenuItems;
		std::deque<InGuiWindow*>   WindowQueue;
		std::vector<Ref<Texture>>  CustomTextures;
	};
}