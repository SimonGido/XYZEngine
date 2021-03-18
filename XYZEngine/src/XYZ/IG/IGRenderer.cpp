#include "stdafx.h"
#include "IGRenderer.h"
#include "IGUIElements.h"
#include "IGAllocator.h"

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
		SubTextures[DockSpace] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(0, 0), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));

		ScissorBuffer = ShaderStorageBuffer::Create(MaxNumberOfScissors * sizeof(IGScissor));

		Colors[DefaultColor] = glm::vec4(1.0f);
		Colors[HooverColor] = glm::vec4(0.5f, 0.7f, 1.0f, 1.0f);
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