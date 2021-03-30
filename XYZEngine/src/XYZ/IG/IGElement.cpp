#include "stdafx.h"
#include "IGElement.h"

#include "IGAllocator.h"
#include "IGUIElements.h"

namespace XYZ {
	
	namespace Helper {
		static bool ResolvePosition(size_t oldQuadCount, const glm::vec2& genSize, IGElement* element, const IGElement& root, IGMesh& mesh, glm::vec2& offset, float& highestInRow)
		{
			if (root.Style.AutoPosition)
			{
				float xBorder = root.Size.x - root.Style.Layout.RightPadding;
				float yBorder = root.Size.y - root.Style.Layout.BottomPadding - root.Style.Layout.TopPadding - IGWindow::PanelHeight;

				if (offset.x + genSize.x > xBorder)
				{
					if (offset.y + genSize.y > yBorder)
					{
						mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
						return false;
					}
					else if (!root.Style.NewRow)
					{
						offset.x += genSize.x + root.Style.Layout.SpacingX;
						mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
						return false;
					}
					else
					{
						offset.x =  root.Style.Layout.LeftPadding;
						offset.y += root.Style.Layout.SpacingY + highestInRow;
						highestInRow = 0.0f;
						// It is generally bigger than xBorder erase it
						if (offset.x + genSize.x > xBorder)
						{
							mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
							return false;
						}
						for (size_t i = oldQuadCount; i < mesh.Quads.size(); ++i)
						{
							mesh.Quads[i].Position.x += offset.x - element->Position.x;
							mesh.Quads[i].Position.y += offset.y - element->Position.y;
						}
					}
				}		
			}
			element->Position = offset;
			offset.x += genSize.x + root.Style.Layout.SpacingX;

			return true;
		}
		static bool IsInside(const glm::vec2& parentPos, const glm::vec2& parentSize, const glm::vec2& pos, const glm::vec2& size)
		{
			return !(pos.x < parentPos.x || pos.x + size.x > parentPos.x + parentSize.x
				|| pos.y < parentPos.y || pos.y + size.y > parentPos.y + parentSize.y);
		}
	}


	IGElement::IGElement(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, IGElementType type)
		:
		Position(position), 
		Size(size), 
		Color(color),
		FrameColor(color),
		ElementType(type)
	{
	}

	glm::vec2 IGElement::BuildMesh(IGMesh& mesh, IGRenderData& renderData, IGPool& pool, const IGElement& root, uint32_t scissorIndex)
	{
		if (Active && ActiveChildren)
		{			
			glm::vec2 offset = {
				Style.Layout.LeftPadding,
				Style.Layout.TopPadding + IGWindow::PanelHeight
			};

			glm::vec2 oldOffset = offset;
			float highestInRow = 0.0f;

			bool outOfRange = false;
			pool.GetHierarchy().TraverseNodeChildren(ID, [&](void* parent, void* child) -> bool {

				IGElement* childElement = static_cast<IGElement*>(child);
				if (childElement->Active)
				{
					// Previous element was out of range and erased , turn off listening to input
					if (outOfRange)
					{
						childElement->ListenToInput = false;
						return false;
					}
					size_t oldQuadCount = mesh.Quads.size();
					glm::vec2 genSize = childElement->GenerateQuads(mesh, renderData);
					if (Helper::ResolvePosition(oldQuadCount, genSize, childElement, root, mesh, offset, highestInRow))
					{
						childElement->ListenToInput = Helper::IsInside(root.GetAbsolutePosition(), root.Size, childElement->GetAbsolutePosition(), childElement->Size);
						highestInRow = std::max(genSize.y, highestInRow);
						offset += childElement->BuildMesh(mesh, renderData, pool, root);
					}
					else
					{
						outOfRange = true;
						childElement->ListenToInput = false;
					}
				}
				return false;
			});


			glm::vec2 result = offset - oldOffset;
			result.y += highestInRow;
			return result;
		}	
		return glm::vec2(0.0f);
	}

	glm::vec2 IGElement::GetAbsolutePosition() const
	{
		if (Parent)
		{
			return Parent->GetAbsolutePosition() + Position;
		}
		return Position;
	}

	IGReturnType IGElement::getAndRestartReturnType()
	{
		IGReturnType old = ReturnType;
		ReturnType = IGReturnType::None;
		return old;
	}
}