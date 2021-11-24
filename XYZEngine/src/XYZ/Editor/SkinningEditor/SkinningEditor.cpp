#include "stdafx.h"
#include "SkinningEditor.h"

#include "XYZ/Asset/AssetManager.h"
#include "XYZ/Renderer/Renderer.h"

#include <imgui.h>

namespace XYZ {
	namespace Editor {
		SkinningEditor::SkinningEditor()
		{
			m_Shader = AssetManager::GetAsset<Shader>(AssetManager::GetAssetHandle("Assets/Shaders/Editor/SkinningEditor.glsl.shader"));
			
			FramebufferSpecification specs;
			specs.ClearColor = { 0.0f,0.0f,0.0f,0.0f };
			specs.Attachments = {
				FramebufferTextureSpecification(ImageFormat::RGBA16F),
				FramebufferTextureSpecification(ImageFormat::DEPTH24STENCIL8)
			};
			m_Pass = RenderPass::Create({ Framebuffer::Create(specs) });
		}
		SkinningEditor::~SkinningEditor()
		{
		}
		void SkinningEditor::OnImGuiRender()
		{
			if (ImGui::Begin("Skinning Editor"))
			{
				if (m_Context.Raw())
				{
					//Renderer::BeginRenderPass(m_Pass, true);


					//Renderer::EndRenderPass();
				}
			}
			ImGui::End();
		}
		void SkinningEditor::SetContext(const Ref<SubTexture>& context)
		{
			m_Context = context;
		}
	}
}