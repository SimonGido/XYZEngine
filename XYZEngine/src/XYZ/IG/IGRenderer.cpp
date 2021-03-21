#include "stdafx.h"
#include "IGRenderer.h"
#include "IGUIElements.h"
#include "IGAllocator.h"

namespace XYZ {
	namespace Helper {
		static bool ResolvePosition(size_t oldQuadCount, const glm::vec2& genSize, IGElement* element, IGElement* parent, IGMesh& mesh, glm::vec2& offset, float& highestInRow, const glm::vec2& rootBorder)
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
				if (element->GetAbsolutePosition().y + genSize.y > rootBorder.y)
				{
					mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
					return false;
				}
				offset.x += genSize.x + parent->Style.Layout.SpacingX;
			}
			
			return true;
		}
	}

	

	glm::vec4 IGRenderData::Colors[IGRenderData::NumColors];

	IGRenderData::IGRenderData()
	{
		Ref<Shader> shader = Shader::Create("Assets/Shaders/InGuiShader.glsl");
		Texture = Texture2D::Create({ TextureWrap::Clamp, TextureParam::Linear, TextureParam::Nearest }, "Assets/Textures/Gui/TexturePack_Dark.png");	
		Font = Ref<XYZ::Font>::Create(14, "Assets/Fonts/arial.ttf");


		Material = Ref<XYZ::Material>::Create(shader);
		Material->Set("u_Texture", Texture, TextureID);
		Material->Set("u_Texture", Font->GetTexture(), FontTextureID);
		Material->Set("u_Color", glm::vec4(1.0f));

		float divisor = 8.0f;
		SubTextures[Button] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(0, 0), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTextures[CheckboxChecked] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(1, 1), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTextures[CheckboxUnChecked] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(0, 1), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTextures[Slider] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(0, 0), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTextures[SliderHandle] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(1, 2), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTextures[Window] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(0, 3), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTextures[MinimizeButton] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(1, 3), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTextures[CloseButton] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(2, 0), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTextures[DownArrow] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(2, 3), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTextures[RightArrow] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(2, 2), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTextures[LeftArrow] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(3, 2), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTextures[Pause] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(2, 1), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTextures[White] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(3, 0), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));

		ScissorBuffer = ShaderStorageBuffer::Create(MaxNumberOfScissors * sizeof(IGScissor));

		Colors[DefaultColor] = glm::vec4(1.0f);
		Colors[HooverColor] = glm::vec4(0.5f, 0.7f, 1.0f, 1.0f);
	}


	static glm::vec2 RebuildMeshRecursive(IGElement* parentElement, IGPool& pool, IGMesh& mesh, IGRenderData& data, const glm::vec2& rootBorder)
	{
		if (parentElement->Active && parentElement->ActiveChildren)
		{			
			glm::vec2 offset = {
				parentElement->Style.Layout.LeftPadding,
				parentElement->Style.Layout.TopPadding + IGWindow::PanelHeight
			};
			glm::vec2 oldOffset = offset;
			float highestInRow = 0.0f;

			bool outOfRange = false;
			pool.GetHierarchy().TraverseNodeChildren(parentElement->GetID(), [&](void* parent, void* child) -> bool {
				
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
					glm::vec2 genSize = childElement->GenerateQuads(mesh, data);
					if (Helper::ResolvePosition(oldQuadCount, genSize, childElement, parentElement, mesh, offset, highestInRow, rootBorder))
					{
						childElement->ListenToInput = true;
						if (genSize.y > highestInRow)
							highestInRow = genSize.y;
						offset += RebuildMeshRecursive(childElement, pool, mesh, data, rootBorder);
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

	void IGRenderData::RebuildMesh(IGAllocator& allocator, IGMesh& mesh)
	{
		if (Rebuild)
		{
			mesh.Quads.clear();
			mesh.Lines.clear();

			for (auto& pool : allocator.GetPools())
			{
				for (int32_t id : pool.GetRootElementIDs())
				{
					IGElement* parentElement = static_cast<IGElement*>(pool.GetHierarchy().GetData(id));
					parentElement->GenerateQuads(mesh, *this);
					RebuildMeshRecursive(parentElement, pool, mesh, *this, parentElement->Position + parentElement->Size);
				}
			}
			Rebuild = false;
		}
	}
}