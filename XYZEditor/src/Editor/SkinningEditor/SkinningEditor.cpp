#include "stdafx.h"
#include "SkinningEditor.h"

#include "EditorLayer.h"

#include <imgui/imgui.h>

namespace XYZ {
	namespace Editor {
		static glm::vec2 GetMouseViewportSpace()
		{
			auto [mx, my] = Input::GetMousePosition();
			auto [winPosX, winPosY] = Input::GetWindowPosition();
			mx -= ImGui::GetWindowPos().x;
			my -= ImGui::GetWindowPos().y;
			mx += winPosX;
			my += winPosY;

			const auto viewportWidth = ImGui::GetWindowSize().x;
			const auto viewportHeight = ImGui::GetWindowSize().y;

			return { (mx / viewportWidth) * 2.0f - 1.0f, ((my / viewportHeight) * 2.0f - 1.0f) * -1.0f };
		}

		SkinningEditor::SkinningEditor(std::string name)
			:
			EditorPanel(std::move(name)),
			m_Camera(30.0f, 1.778f, 0.1f, 1000.0f)
		{	
			m_Camera.LockOrtho(true);

			m_CommandBuffer = RenderCommandBuffer::Create(0, "SkinningEditor");
			m_CommandBuffer->CreateTimestampQueries(GPUTimeQueries::Count());

			Ref<MaterialAsset> quadMaterial = Ref<MaterialAsset>::Create(Renderer::GetDefaultResources().DefaultQuadMaterial->GetShaderAsset());
			Ref<MaterialAsset> lineMaterial = Ref<MaterialAsset>::Create(Renderer::GetDefaultResources().DefaultLineMaterial->GetShaderAsset());
			Ref<MaterialAsset> circleMaterial = Ref<MaterialAsset>::Create(Renderer::GetDefaultResources().DefaultCircleMaterial->GetShaderAsset());

			m_Renderer2D = Ref<Renderer2D>::Create(m_CommandBuffer, quadMaterial, lineMaterial, circleMaterial, nullptr);
			
		
			Ref<ShaderAsset> meshAnimShaderAsset = AssetManager::GetAsset<ShaderAsset>("Resources/Shaders/AnimMeshShader.shader");
			m_MaterialAsset = Ref<MaterialAsset>::Create(meshAnimShaderAsset);
			m_MaterialAsset->SetTexture("u_Texture", Renderer::GetDefaultResources().WhiteTexture);
			
			m_Texture = Texture2D::Create("Assets/Textures/full_simple_char.png");
			m_Context = Ref<SubTexture>::Create(m_Texture);

			for (uint32_t i = 0; i < Renderer2D::GetMaxTextures(); ++i)
				quadMaterial->SetTexture("u_Texture", m_Context->GetTexture(), i);
		}
		void SkinningEditor::OnImGuiRender(bool& open)
		{
			if (ImGui::Begin("Skinning Editor", &open))
			{
				const ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
				m_Hoovered = ImGui::IsWindowHovered();
				m_Focused = ImGui::IsWindowFocused();
				if (m_Hoovered && m_Focused)
				{
					glm::vec4 pos = m_Camera.GetProjectionMatrix() * glm::vec4(GetMouseViewportSpace(), 0.0f, 1.0f);
					if (Input::IsMouseButtonPressed(MouseCode::MOUSE_BUTTON_LEFT))
						m_Points.push_back(glm::vec2(pos.x * m_Camera.GetAspectRatio() , pos.y));
				}
				if (viewportPanelSize.x != m_ViewportSize.x || viewportPanelSize.y != m_ViewportSize.y)
				{
					m_ViewportSize = viewportPanelSize;
					m_UpdateViewport = true;
				}
				UI::Image(m_Renderer2D->GetTargetRenderPass()->GetSpecification().TargetFramebuffer->GetImage(), viewportPanelSize);
			
			}
			ImGui::End();
		}
		void SkinningEditor::OnUpdate(Timestep ts)
		{
			if (m_UpdateViewport)
			{
				m_UpdateViewport = false;
				m_Camera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
				m_Renderer2D->GetTargetRenderPass()->GetSpecification().TargetFramebuffer->Resize(m_ViewportSize.x, m_ViewportSize.y);
			}
			if (m_Hoovered && m_Focused)
			{
				m_Camera.OnUpdate(ts);
			}
			m_CommandBuffer->Begin();
			m_GPUTimeQueries.GPUTime = m_CommandBuffer->BeginTimestampQuery();
			m_Renderer2D->BeginScene(m_Camera.GetViewProjection(), m_Camera.GetViewMatrix(), true);
	
			const uint32_t width = m_Texture->GetWidth();
			const uint32_t height = m_Texture->GetHeight();
			m_Renderer2D->SubmitQuad(glm::vec3(0.0f), glm::vec2(5.0f), m_Context->GetTexCoords(), 0, glm::vec4(1.0f));

			for (auto p : m_Points)
				m_Renderer2D->SubmitCircle(glm::vec3(p, 0.0f), 0.2f, 10);

			m_Renderer2D->EndScene();
			m_CommandBuffer->EndTimestampQuery(m_GPUTimeQueries.GPUTime);

			m_CommandBuffer->End();
			m_CommandBuffer->Submit();
		}
		void SkinningEditor::SetSceneContext(const Ref<Scene>& scene)
		{
			m_Scene = scene;
		}
	
		bool SkinningEditor::OnEvent(Event& e)
		{
			if (m_Hoovered && m_Focused)
				m_Camera.OnEvent(e);
			return false;
		}
		void SkinningEditor::SetContext(const Ref<SubTexture>& subTexture)
		{
			m_Context = subTexture;
			m_Mesh.Reset();
		}
	}
}

