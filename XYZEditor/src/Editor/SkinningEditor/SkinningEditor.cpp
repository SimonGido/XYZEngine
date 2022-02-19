#include "stdafx.h"
#include "SkinningEditor.h"

#include "EditorLayer.h"
#include "Editor/EditorHelper.h"

#include "XYZ/ImGui/ImGui.h"

#include <imgui/imgui.h>

#include <delaunator/include/delaunator.hpp>

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
			m_CameraController(16.0f / 9.0f),
			m_ButtonSize(25.0f)
		{	
			m_CommandBuffer = RenderCommandBuffer::Create(0, "SkinningEditor");
			m_CommandBuffer->CreateTimestampQueries(GPUTimeQueries::Count());

			Ref<MaterialAsset> quadMaterial = Ref<MaterialAsset>::Create(Renderer::GetDefaultResources().DefaultQuadMaterial->GetShaderAsset());
			Ref<MaterialAsset> lineMaterial = Ref<MaterialAsset>::Create(Renderer::GetDefaultResources().DefaultLineMaterial->GetShaderAsset());
			Ref<MaterialAsset> circleMaterial = Ref<MaterialAsset>::Create(Renderer::GetDefaultResources().DefaultCircleMaterial->GetShaderAsset());

			m_Renderer2D = Ref<Renderer2D>::Create(m_CommandBuffer, quadMaterial, lineMaterial, circleMaterial, nullptr);	
			m_BoneTransformsStorageSet = StorageBufferSet::Create(Renderer::GetConfiguration().FramesInFlight);
			m_BoneTransformsStorageSet->Create(1 * sizeof(RenderQueue::BoneTransforms), 2, 0);
			m_TransformsVertexBuffer = VertexBuffer::Create(1 * sizeof(glm::mat4));

			
			m_Texture = Texture2D::Create("Assets/Textures/full_simple_char.png");
			m_Context = Ref<SubTexture>::Create(m_Texture);
			SetContext(m_Context);

			Ref<ShaderAsset> meshAnimShaderAsset = AssetManager::GetAsset<ShaderAsset>("Resources/Shaders/AnimMeshShader.shader");
			m_MaterialAsset = Ref<MaterialAsset>::Create(meshAnimShaderAsset);
			m_MaterialAsset->SetTexture("u_Texture", m_Context->GetTexture());


			for (uint32_t i = 0; i < Renderer2D::GetMaxTextures(); ++i)
				quadMaterial->SetTexture("u_Texture", m_Context->GetTexture(), i);

			m_Colors[Points] = glm::vec4(0.4f, 0.6f, 0.8f, 1.0f);
			m_Colors[Selected] = glm::vec4(0.6f, 0.8f, 1.0f, 1.0f);
			m_Colors[Lines] = glm::vec4(0.7f);

			createPipeline();
		}
		void SkinningEditor::OnImGuiRender(bool& open)
		{
			UI::ScopedStyleStack styleStack(true, ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			if (ImGui::Begin("Skinning Editor", &open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				const ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
				m_MousePosition = getMouseWorldPosition();
				m_Hoovered = ImGui::IsWindowHovered();
				m_Focused = ImGui::IsWindowFocused();
				
				if (viewportPanelSize.x != m_ViewportSize.x 
				 || viewportPanelSize.y != m_ViewportSize.y)
				{
					m_ViewportSize = viewportPanelSize;
					m_UpdateViewport = true;
				}
				
				UI::Image(m_Renderer2D->GetTargetRenderPass()->GetSpecification().TargetFramebuffer->GetImage(), viewportPanelSize);
				
				bool handled = handleToolbar();
				if (m_Hoovered && m_Focused && !handled)
				{
					handleStateActions();
				}
			}
			ImGui::End();
		}
		void SkinningEditor::OnUpdate(Timestep ts)
		{
			if (m_UpdateViewport)
			{
				m_UpdateViewport = false;
				m_CameraController.SetAspectRatio(m_ViewportSize.x / m_ViewportSize.y);
				m_Renderer2D->GetTargetRenderPass()->GetSpecification().TargetFramebuffer->Resize(m_ViewportSize.x, m_ViewportSize.y);
			}
			if (m_Hoovered && m_Focused)
			{
				m_CameraController.OnUpdate(ts);
			}
			auto& camera = m_CameraController.GetCamera();
			m_CommandBuffer->Begin();
	
			m_GPUTimeQueries.GPUTime = m_CommandBuffer->BeginTimestampQuery();
			m_Renderer2D->BeginScene(camera.GetViewProjectionMatrix(), camera.GetViewMatrix(), true);
	
			// TODO: it is flickering because, Storage with bones is updater multiple times per frame ( it is global inside shaders )
			if (m_Mesh.Raw())
			{
				Renderer::BindPipeline(
					m_CommandBuffer,
					m_Pipeline,
					m_Renderer2D->GetCameraBufferSet(),
					m_BoneTransformsStorageSet,
					m_MaterialAsset->GetMaterial()
				);

				Renderer::RenderMesh(
					m_CommandBuffer,
					m_Pipeline,
					m_MaterialAsset->GetMaterialInstance(),
					m_Mesh->GetVertexBuffer(),
					m_Mesh->GetIndexBuffer(),
					0,
					m_TransformsVertexBuffer,
					0,
					1
				);
			}

			const uint32_t width = m_Texture->GetWidth();
			const uint32_t height = m_Texture->GetHeight();
			//m_Renderer2D->SubmitQuad(
			//	glm::vec3(0.0f), glm::vec2(1.0f * m_ContextAspectRatio, 1.0f), 
			//	m_Context->GetTexCoords(), 0, glm::vec4(1.0f)
			//);
			
			if (m_State == State::PlacingPoints)
				m_Renderer2D->SubmitFilledCircle(glm::vec3(m_MousePosition, 0.0f), glm::vec2(0.03f), 0.5f, m_Colors[Points]);
			for (auto p : m_Points)
				m_Renderer2D->SubmitFilledCircle(glm::vec3(p, 0.0f), glm::vec2(0.03f), 0.5f, m_Colors[Points]);

			for (auto& triangle : m_Triangles)
			{
				glm::vec3 p0 = glm::vec3(m_Points[triangle.First], 0.0f);
				glm::vec3 p1 = glm::vec3(m_Points[triangle.Second], 0.0f);
				glm::vec3 p2 = glm::vec3(m_Points[triangle.Third], 0.0f);

				m_Renderer2D->SubmitLine(p0, p1, m_Colors[Lines]);
				m_Renderer2D->SubmitLine(p1, p2, m_Colors[Lines]);
				m_Renderer2D->SubmitLine(p2, p0, m_Colors[Lines]);
			}



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
				m_CameraController.OnEvent(e);
			return false;
		}
		void SkinningEditor::SetContext(const Ref<SubTexture>& subTexture)
		{
			m_Context = subTexture;
			const float width = m_Context->GetTexCoords().z - m_Context->GetTexCoords().x;
			const float height = m_Context->GetTexCoords().w - m_Context->GetTexCoords().y;

			m_ContextAspectRatio = width / height;
			m_Mesh = MeshFactory::CreateQuad(glm::vec2(m_ContextAspectRatio, 1.0f), m_Context->GetTexCoords());
		
			glm::mat4 bone(1.0f);
			m_BoneTransformsStorageSet->Update(&bone, sizeof(glm::mat4), 0, 0, 2);
			m_TransformsVertexBuffer->Update(&bone, sizeof(glm::mat4));
		}
		bool SkinningEditor::handleToolbar()
		{
			bool handled = false;

			UI::Toolbar(glm::vec2(8.0f, 8.0f), glm::vec2(0.0f, 0.0f), false,
				[&]() {
				if (ImGui::Button("Points"))
				{
					m_State = State::PlacingPoints;
				}
				handled |= ImGui::IsItemClicked(ImGuiMouseButton_Left);
				},
				[&]() {
				if (ImGui::Button("Triangulate"))
				{
					m_State = State::None;
					if (m_Points.size() >= 3)
					{
						std::vector<double> coords;
						for (auto p : m_Points)
						{
							coords.push_back(p.x);
							coords.push_back(p.y);
						}
						m_Triangles.clear();
						delaunator::Delaunator del(coords);
						for (size_t i = 0; i < del.triangles.size(); i += 3)
						{
							m_Triangles.push_back({
								static_cast<uint32_t>(del.triangles[i]),
								static_cast<uint32_t>(del.triangles[i + 1]),
								static_cast<uint32_t>(del.triangles[i + 2])
								});
						}
					}
				}
				handled |= ImGui::IsItemClicked(ImGuiMouseButton_Left);
				},
				[&]() {
				if (ImGui::Button("Edit"))
				{
					m_State = State::None;
					m_Triangles.clear();
					m_Points.clear();
				}
				handled |= ImGui::IsItemClicked(ImGuiMouseButton_Left);
				}
			);
			return handled;
		}
		void SkinningEditor::handleStateActions()
		{
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && m_State == State::PlacingPoints)
			{
				m_Points.push_back(m_MousePosition);
			}
		}
		void SkinningEditor::createPipeline()
		{
			Ref<Shader> shader = m_MaterialAsset->GetShader();
			
			PipelineSpecification spec;
			spec.Layouts = shader->GetLayouts();
			spec.RenderPass = m_Renderer2D->GetTargetRenderPass();
			spec.Shader = shader;
			spec.Topology = PrimitiveTopology::Triangles;

			m_Pipeline = Pipeline::Create(spec);
		}
		glm::vec2 SkinningEditor::getMouseWorldPosition() const
		{
			auto [x, y] = Input::GetMousePosition();
			auto width = ImGui::GetWindowWidth();
			auto height = ImGui::GetWindowHeight();
	
			x -= ImGui::GetWindowPos().x;
			y -= ImGui::GetWindowPos().y;

			auto bounds = m_CameraController.GetBounds();
			auto pos = m_CameraController.GetCamera().GetPosition();
			x = (x / width) * bounds.GetWidth() - bounds.GetWidth() * 0.5f;
			y = bounds.GetHeight() * 0.5f - (y / height) * bounds.GetHeight();
			
			return { x + pos.x, y + pos.y };
		}
	}
}

