#include "stdafx.h"
#include "InGuiFrame.h"

#include "InGui.h"

namespace XYZ {
	InGuiFrame::InGuiFrame()
		:
		CurrentWindow(nullptr),
		MovedWindowOffset(0.0f),
		CurrentMenuWidth(0.0f),
		HighlightNext(false),
		CurrentTreeDepth(0)
	{
		
	}
	InGuiFrame::~InGuiFrame()
	{
		XYZ_ASSERT(WindowQueue.empty(), "Window queue is not empty, forgot end?");
	}
	uint32_t InGuiFrame::AddCustomTexture(const Ref<Texture>& texture)
	{
		const InGuiConfig& config= InGui::GetContext().m_Config;
		if (texture->GetRendererID() == config.Texture->GetRendererID())
			return InGuiConfig::DefaultTextureIndex;
		if (texture->GetRendererID() == config.WhiteTexture->GetRendererID())
			return InGuiConfig::WhiteTextureIndex;

		uint32_t counter = 0;
		for (auto& tex : CustomTextures)
		{
			if (tex->GetRendererID() == texture->GetRendererID())
				return (counter + 1) + InGuiConfig::FontTextureIndex;
			counter++;
		}
		CustomTextures.push_back(texture);
		return (uint32_t)CustomTextures.size() + InGuiConfig::FontTextureIndex;
	}
}