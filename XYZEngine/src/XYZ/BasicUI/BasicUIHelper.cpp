#include "stdafx.h"
#include "BasicUIHelper.h"
#include "BasicUI.h"

namespace XYZ {
	glm::vec2 bUIHelper::FindTextSize(const char* source, const Ref<Font>& font)
	{
		if (!source)
			return { 0.0f, 0.0f };

		float width = 0.0f;
		float xCursor = 0.0f;
		float yCursor = 0.0f;

		uint32_t counter = 0;
		while (source[counter] != '\0')
		{
			auto& character = font->GetCharacter(source[counter]);
			if (source[counter] == '\n')
			{
				width = xCursor;
				yCursor += font->GetLineHeight();
				xCursor = 0.0f;
				counter++;
				continue;
			}
			xCursor += character.XAdvance;
			counter++;
		}
		if (width < xCursor)
			width = xCursor;
		return { width, yCursor + font->GetLineHeight() };
	}
	void bUIHelper::ResolvePosition(int32_t elementID, Tree& tree, const bUILayout& layout)
	{
		glm::vec2 offset(layout.LeftOffset, layout.TopOffset);
		bUIElement* parentElement = static_cast<bUIElement*>(tree.GetData(elementID));
		glm::vec2 border(
			parentElement->Size.x - layout.RightOffset, 
			parentElement->Size.y
		);
		float highestInRow = 0.0f;
		tree.TraverseNodeChildren(elementID, [&](void* parent, void* child)->bool {

			bUIElement* childElement = static_cast<bUIElement*>(child);
			glm::vec2 textSize = FindTextSize(childElement->Label.c_str(), bUI::GetConfig().GetFont());
			float xOffset = childElement->Size.x + textSize.x;
			if (offset.x + xOffset > border.x)
			{
				offset.x = layout.LeftOffset;
				offset.y += highestInRow + layout.SpacingY;
				highestInRow = 0.0f;
			}
			float height = std::max(childElement->Size.y, textSize.y);
			highestInRow = std::max(highestInRow, height);
			childElement->Coords = offset;
			offset.x += xOffset + layout.SpacingX;
			return false;
		});
	}
	bool bUIHelper::IsInside(const glm::vec2& posA, const glm::vec2& sizeA, const glm::vec2& posB, const glm::vec2& sizeB)
	{
		// A inside B	
		return (posB.x		     < posA.x
			&&  posB.x + sizeB.x > posA.x + sizeA.x
			&&  posB.y			 < posA.y
			&&  posB.y + sizeB.y > posA.y + sizeA.y
			);
	}
}