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
					float yBorder = parent->Size.y - parent->Style.Layout.BottomPadding - parent->Style.Layout.TopPadding - IGWindow::PanelHeight;
					
					if (offset.x + genSize.x > xBorder)
					{
						if (offset.y + genSize.y > yBorder)
						{
							mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
							return false;
						}
						else if (!parent->Style.NewRow)
						{
							offset.x += genSize.x + parent->Style.Layout.SpacingX;
							mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
							return false;
						}
						else
						{
							offset.x = parent->Style.Layout.LeftPadding;
							offset.y += parent->Style.Layout.SpacingY + highestInRow;
						}						
					}
					
				}
				element->Position = offset;
				offset.x += genSize.x + parent->Style.Layout.SpacingX;
			}
			
			return true;
		}
	}

	static void RebuildMeshRecursive(IGElement* parentElement, IGPool& pool, IGMesh& mesh, IGRenderData& data)
	{
		if (parentElement->Active && parentElement->ActiveChildren)
		{
			glm::vec2 offset = {
				parentElement->Style.Layout.LeftPadding,
				parentElement->Style.Layout.TopPadding + IGWindow::PanelHeight
			};
			float highestInRow = 0.0f;

			pool.GetHierarchy().TraverseNodeChildren(parentElement->GetID(), [&](void* parent, void* child) -> bool {

				parentElement = static_cast<IGElement*>(parent);
				IGElement* childElement = static_cast<IGElement*>(child);
				if (childElement->Active)
				{
					size_t oldQuadCount = mesh.Quads.size();
					glm::vec2 genSize = childElement->GenerateQuads(mesh, data);
					if (Helper::ResolvePosition(oldQuadCount, genSize, childElement, parentElement, mesh, offset, highestInRow))
					{
						if (genSize.y > highestInRow)
							highestInRow = genSize.y;

						RebuildMeshRecursive(childElement, pool, mesh, data);
					}
					else
					{
						return true;
					}
				}
				return false;
			});
		}	
	}

	void IGRenderData::RebuildMesh(IGAllocator& allocator, IGMesh& mesh)
	{
		for (auto& pool : allocator.GetPools())
		{		
			for (int32_t id : pool.GetRootElementIDs())
			{
				IGElement* parentElement = static_cast<IGElement*>(pool.GetHierarchy().GetData(id));
				parentElement->GenerateQuads(mesh, *this);
				RebuildMeshRecursive(parentElement, pool, mesh, *this);
			}
		}
	}
}