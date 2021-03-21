#include "stdafx.h"
#include "IGRenderer.h"
#include "IGUIElements.h"
#include "IGAllocator.h"

namespace XYZ {
	
	glm::vec4 IGRenderData::Colors[IGRenderData::NumColors];

	IGRenderData::IGRenderData()
	{
		Texture = Texture2D::Create({ TextureWrap::Clamp, TextureParam::Linear, TextureParam::Nearest }, "Assets/Textures/Gui/TexturePack_Dark.png");	
		Font = Ref<XYZ::Font>::Create(14, "Assets/Fonts/arial.ttf");


		DefaultMaterial = Ref<XYZ::Material>::Create(Shader::Create("Assets/Shaders/InGuiShader.glsl"));
		DefaultMaterial->Set("u_Texture", Texture, TextureID);
		DefaultMaterial->Set("u_Texture", Font->GetTexture(), FontTextureID);
		DefaultMaterial->Set("u_Color", glm::vec4(1.0f));

		ScissorMaterial = Ref<XYZ::Material>::Create(Shader::Create("Assets/Shaders/ScissorShader.glsl"));
		ScissorMaterial->Set("u_Texture", Texture, TextureID);
		ScissorMaterial->Set("u_Texture", Font->GetTexture(), FontTextureID);
		ScissorMaterial->Set("u_Color", glm::vec4(1.0f));

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

	void IGRenderData::RebuildMesh(IGAllocator& allocator)
	{
		if (Rebuild)
		{
			rebuildMesh(allocator);
			Rebuild = false;
			if (RebuildTwice)
			{
				rebuildMesh(allocator);
				RebuildTwice = false;
			}

			if (Scissors.size())
			{
				ScissorBuffer->Update(Scissors.data(), Scissors.size() * sizeof(IGScissor));
				ScissorBuffer->BindRange(0, Scissors.size() * sizeof(IGScissor), 0);
			}
		}
	}
	void IGRenderData::rebuildMesh(IGAllocator& allocator)
	{
		Mesh.Quads.clear();
		Mesh.Lines.clear();
		ScrollableMesh.Quads.clear();
		ScrollableMesh.Lines.clear();

		Scissors.clear();
		for (auto& pool : allocator.GetPools())
		{
			for (int32_t id : pool.GetRootElementIDs())
			{
				IGElement* parentElement = static_cast<IGElement*>(pool.GetHierarchy().GetData(id));
				parentElement->GenerateQuads(Mesh, *this);
				parentElement->BuildMesh(Mesh, *this, pool, parentElement->Position + parentElement->Size);
			}
		}
	}
}