#include "stdafx.h"
#include "InGuiRenderer.h"
#include "InGuiUIElements.h"
#include "InGuiAllocator.h"

namespace XYZ {
	namespace Helper {
		static bool ResolvePosition(size_t oldQuadCount, const glm::vec2& genSize, IGElement* element, IGElement* parent, IGMesh& mesh, glm::vec2& offset, float highestInRow)
		{
			if (parent)
			{
				if (parent->Style.AutoPosition)
				{
					float xBorder = parent->Size.x - parent->Style.Layout.RightPadding;
					float yBorder = parent->Size.y - parent->Style.Layout.BottomPadding;
					if (offset.x + genSize.x > xBorder)
					{
						if (!parent->Style.NewRow)
						{
							offset.x += genSize.x + parent->Style.Layout.SpacingX;
							mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
							return false;
						}
						else if (offset.y + genSize.y < yBorder)
						{
							float oldX = mesh.Quads[oldQuadCount].Position.x;
							mesh.Quads[oldQuadCount].Position.x = parent->Style.Layout.LeftPadding + parent->GetAbsolutePosition().x;
							mesh.Quads[oldQuadCount].Position.y += highestInRow + parent->Style.Layout.SpacingY + offset.y;
							for (size_t i = oldQuadCount + 1; i < mesh.Quads.size(); ++i)
							{
								float diff = mesh.Quads[i].Position.x - oldX;
								oldX = mesh.Quads[i].Position.x;

								mesh.Quads[i].Position.x = mesh.Quads[i - 1].Position.x + diff;
								mesh.Quads[i].Position.y += highestInRow + parent->Style.Layout.SpacingY + offset.y;
							}

							offset.x = parent->Style.Layout.LeftPadding;
							offset.y += parent->Style.Layout.SpacingY + highestInRow;
							highestInRow = 0.0f;
						}
						else
						{
							mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
							return false;
						}
					}
					else
					{
						element->Position = offset;
						offset.x += genSize.x + parent->Style.Layout.SpacingX;
					}
				}
			}
			return true;
		}
	}
	void IGRenderData::RebuildMesh(IGAllocator& allocator, IGMesh& mesh)
	{
		for (auto& pool : allocator.GetPools())
		{
			for (int32_t id : pool.GetParentIDs())
			{
				IGElement* parentElement = static_cast<IGElement*>(pool.GetHierarchy().GetData(id));
				parentElement->GenerateQuads(mesh, *this);
				if (parentElement->Active)
				{
					glm::vec2 offset = {
						parentElement->Style.Layout.LeftPadding,
						parentElement->Style.Layout.TopPadding + IGWindow::PanelHeight
					};
					float highestInRow = 0.0f;

					pool.GetHierarchy().TraverseNodeChildren(id, [&](void* parent, void* child) -> bool {

						parentElement = static_cast<IGElement*>(parent);
						IGElement* childElement = static_cast<IGElement*>(child);
						if (!childElement->Active)
							return false;

						size_t oldQuadCount = mesh.Quads.size();
						glm::vec2 genSize = childElement->GenerateQuads(mesh, *this);
						Helper::ResolvePosition(oldQuadCount, genSize, childElement, parentElement, mesh, offset, highestInRow);
						if (genSize.y > highestInRow)
							highestInRow = genSize.y;

						
						return false;
						});
				}
			}
		}
	}
}