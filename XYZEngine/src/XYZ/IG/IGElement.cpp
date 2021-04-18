#include "stdafx.h"
#include "IGElement.h"

#include "IGAllocator.h"
#include "IGUIElements.h"
#include "IGHelper.h"

namespace XYZ {
	
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

			glm::vec2 rootBorder = IGHelper::GetBorder(*root);
			glm::vec2 oldOffset = offset;
			float highestInRow = 0.0f;
			bool in = true;
		
			pool.GetHierarchy().TraverseNodeChildren(ID, [&](void* parent, void* child) -> bool {

				IGElement* childElement = static_cast<IGElement*>(child);
				if (childElement->Active)
				{
					// Previous element was out of range and erased , turn off listening to input
					if (!in)
					{
						childElement->ListenToInput = false;
						return false;
					}
					size_t oldQuadCount = mesh.Quads.size();
					size_t oldLineCount = mesh.Lines.size();
					glm::vec2 genSize = childElement->GenerateQuads(mesh, renderData);
					IGHelper::ResolvePosition(rootBorder, genSize, Style, offset, *childElement, mesh, highestInRow, oldQuadCount, oldLineCount);
					in = IGHelper::IsInside(root->GetAbsolutePosition(), root->Size, childElement->GetAbsolutePosition(), childElement->Size, (IGHelper::Right | IGHelper::Bottom));
					childElement->ListenToInput = IGHelper::IsInside(root->GetAbsolutePosition(), root->Size, childElement->GetAbsolutePosition(), childElement->Size, (IGHelper::Left | IGHelper::Right | IGHelper::Top | IGHelper::Bottom));
					if (in)
					{						
						highestInRow = std::max(genSize.y, highestInRow);
						offset += childElement->BuildMesh(root, renderData, pool, mesh, scissorIndex);
					}
					else
					{
						IGHelper::PopFromMesh(mesh, oldQuadCount, oldLineCount);
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

	bool IGElement::Is(IGReturnType returnType)
	{
		if (ReturnType == returnType)
		{
			// Return type is equal so the state was satisfied, set it to none
			ReturnType = IGReturnType::None;
			return true;
		}
		return false;
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

	
}