#include "stdafx.h"
#include "IGElement.h"

#include "IGAllocator.h"
#include "IGUIElements.h"

namespace XYZ {
	
	namespace Helper {
		static bool ResolvePosition(
			IGElement* element, 
			const glm::vec2& border, const glm::vec2& genSize, const IGStyle& style, 
			IGMesh& mesh, glm::vec2& offset, float& maxY, 
			size_t oldQuadCount)
		{		
			if (style.AutoPosition)
			{
				if (offset.x + genSize.x > border.x)
				{
					if (offset.y + genSize.y > border.y)
					{
						mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
						return false;
					}
					else if (!style.NewRow)
					{
						offset.x += genSize.x + style.Layout.SpacingX;
						mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
						return false;
					}
					else
					{
						offset.x = style.Layout.LeftPadding;
						offset.y += style.Layout.SpacingY + maxY;
						maxY = 0.0f;
						// It is generally bigger than xBorder erase it
						if (offset.x + genSize.x > border.x)
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
				element->Position = offset;
				offset.x += genSize.x + style.Layout.SpacingX;
			}
			return true;
		}
		static glm::vec2 Border(IGElement* element)
		{
			float xBorder = element->Size.x - element->Style.Layout.RightPadding;
			float yBorder = element->Size.y - element->Style.Layout.BottomPadding - element->Style.Layout.TopPadding;
			return { xBorder, yBorder };
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

	IGElement::IGElement(IGElement&& other) noexcept
		:
		Position(other.Position),
		Size(other.Size),
		Color(other.Color),
		FrameColor(other.FrameColor),
		Label(std::move(other.Label)),
		Style(other.Style),
		Parent(other.Parent),
		Active(other.Active),
		ActiveChildren(other.ActiveChildren),
		ListenToInput(other.ListenToInput),
		ElementType(other.ElementType),
		ReturnType(other.ReturnType),
		ID(other.ID)
	{
		other.Parent = nullptr;
	}


	glm::vec2 IGElement::BuildMesh(IGElement* root, IGRenderData& renderData, IGPool& pool, IGMesh& mesh, uint32_t scissorIndex)
	{
		if (Active && ActiveChildren)
		{			
			glm::vec2 offset = {
				Style.Layout.LeftPadding,
				Style.Layout.TopPadding
			};

			glm::vec2 rootBorder = Helper::Border(root);
			glm::vec2 oldOffset = offset;
			float highestInRow = 0.0f;

			bool out = false;
		
			pool.GetHierarchy().TraverseNodeChildren(ID, [&](void* parent, void* child) -> bool {

				IGElement* childElement = static_cast<IGElement*>(child);
				if (childElement->Active)
				{
					// Previous element was out of range and erased , turn off listening to input
					if (out)
					{
						childElement->ListenToInput = false;
						return false;
					}
					size_t oldQuadCount = mesh.Quads.size();
					glm::vec2 genSize = childElement->GenerateQuads(mesh, renderData);
					out = !Helper::ResolvePosition(childElement, rootBorder, genSize, root->Style, mesh, offset, highestInRow, oldQuadCount);
					childElement->ListenToInput = Helper::IsInside(root->GetAbsolutePosition(), root->Size, childElement->GetAbsolutePosition(), childElement->Size);
					if (!out)
					{
						highestInRow = std::max(genSize.y, highestInRow);
						offset += childElement->BuildMesh(root, renderData, pool, mesh, scissorIndex);
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

	IGElement* IGElement::FindRoot()
	{
		if (Parent)
		{
			return Parent->FindRoot();
		}
		return this;
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