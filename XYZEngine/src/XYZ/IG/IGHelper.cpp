#include "stdafx.h"
#include "IGHelper.h"

namespace XYZ {
	void IGHelper::PopFromMesh(IGMesh& mesh, size_t startQuad, size_t startLine)
	{
		mesh.Quads.erase(mesh.Quads.begin() + startQuad, mesh.Quads.end());
		mesh.Lines.erase(mesh.Lines.begin() + startLine, mesh.Lines.end());
	}
	void IGHelper::ResolvePosition(const glm::vec2& border, const glm::vec2& genSize, const IGStyle& style, glm::vec2& offset, IGElement& element, IGMesh& mesh, float& maxY, size_t oldQuadCount, size_t oldLineCount)
	{
		if (style.AutoPosition)
		{
			if (offset.x + genSize.x > border.x)
			{
				if (!style.NewRow)
				{
					offset.x += genSize.x + style.Layout.SpacingX;
					PopFromMesh(mesh, oldQuadCount, oldLineCount);
					return;
				}
				else
				{
					offset.x = style.Layout.LeftPadding;
					offset.y += style.Layout.SpacingY + maxY;
					maxY = 0.0f;
					// It is generally bigger than xBorder erase it
					if (offset.x + genSize.x > border.x)
					{
						PopFromMesh(mesh, oldQuadCount, oldLineCount);
						return;
					}
					for (size_t i = oldQuadCount; i < mesh.Quads.size(); ++i)
					{
						mesh.Quads[i].Position.x += offset.x - element.Position.x;
						mesh.Quads[i].Position.y += offset.y - element.Position.y;
					}
				}
			}
			element.Position = offset;
			offset.x += genSize.x + style.Layout.SpacingX;
		}
	}
	glm::vec2 IGHelper::GetBorder(const IGElement& element)
	{
		float xBorder = element.Size.x - element.Style.Layout.RightPadding;
		float yBorder = element.Size.y - element.Style.Layout.BottomPadding - element.Style.Layout.TopPadding;
		return { xBorder, yBorder };
	}
	bool IGHelper::IsInside(const glm::vec2& parentPos, const glm::vec2& parentSize, const glm::vec2& pos, const glm::vec2& size, uint8_t flags)
	{
		if (IS_SET(flags, Left))
		{
			if (pos.x < parentPos.x)
				return false;
		}
		if (IS_SET(flags, Right))
		{
			if (pos.x + size.x > parentPos.x + parentSize.x)
				return false;
		}
		if (IS_SET(flags, Top))
		{
			if (pos.y < parentPos.y)
				return false;
		}
		if (IS_SET(flags, Bottom))
		{
			if (pos.y + size.y > parentPos.y + parentSize.y)
				return false;
		}
		return true;
	}
}