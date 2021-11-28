#include "stdafx.h"
#include "SpriteEditor.h"

#include "XYZ/Asset/AssetManager.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Editor/EditorHelper.h"
#include "XYZ/Utils/StringUtils.h"
#include "XYZ/Utils/FileSystem.h"

#include <imgui.h>


#include <filesystem>

namespace XYZ {
	namespace Editor {

		static glm::vec4 CalcBorders(const Ref<SubTexture>& subTexture)
		{
			glm::vec4 border = subTexture->GetTexCoords();
			auto& texture = subTexture->GetTexture();
			border.x *= texture->GetWidth();
			border.y *= texture->GetHeight();
			border.z *= texture->GetWidth();
			border.w *= texture->GetHeight();
			return border;
		}

		static void SetBorder(Ref<SubTexture>& subTexture, glm::vec4 border)
		{
			auto& texture = subTexture->GetTexture();
		
			border.x /= texture->GetWidth();
			border.y /= texture->GetHeight();
			border.z /= texture->GetWidth();
			border.w /= texture->GetHeight();

			subTexture->SetTexCoords({ border.x, border.y, border.z, border.w });
		}

		SpriteEditor::SpriteEditor()
			:
			m_Camera(30.0f, 1.778f, 0.1f, 1000.0f),
			m_PixelBuffer(nullptr),
			m_ViewportFocused(false),
			m_ViewportHovered(false),
			m_ToolSectionWidth(300.0f),
			m_ViewSectionWidth(300.0f)
		{
			m_SpriteRenderer.Material = AssetManager::GetAsset<Material>(AssetManager::GetAssetHandle("Assets/Materials/Material.mat"));
			FramebufferSpecification specs;
			specs.ClearColor = { 0.1f,0.1f,0.1f,1.0f };
			specs.Attachments = {
				FramebufferTextureSpecification(ImageFormat::RGBA16F),
				FramebufferTextureSpecification(ImageFormat::DEPTH24STENCIL8)
			};
			m_RenderPass = RenderPass::Create({ Framebuffer::Create(specs) });
			m_Camera.LockOrtho(true);
		}
		void SpriteEditor::OnUpdate(Ref<Renderer2D> renderer2D, Ref<Renderer2D> renderer, Timestep ts)
		{
			if (m_Context.Raw())
			{
				if (m_ViewportHovered && m_ViewportFocused)
				{
					m_Camera.OnUpdate(ts);			
				}
				const glm::vec4 border = m_Output->GetTexCoords();
				const glm::vec2 textureSize = { (float)m_Context->GetWidth(), (float)m_Context->GetHeight() };

				const float aspect = textureSize.x / textureSize.y;

				const glm::vec3 min = glm::vec3(glm::vec2((border.x - 0.5f) * aspect, border.y - 0.5f), 0.0f);
				const glm::vec3 max = glm::vec3(glm::vec2((border.z - 0.5f) * aspect, border.w - 0.5f), 0.0f);
				renderer->SubmitAABB(min, max, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
				
				//renderer->SubmitEditorSprite(m_SpriteRenderer.Material, m_SpriteRenderer.SubTexture, m_SpriteRenderer.Color, m_Transform.WorldTransform);
				//renderer->BeginPass(m_RenderPass, m_Camera.GetViewProjection(), m_Camera.GetPosition());
				//renderer->EndPass(renderer2D, true);
			}
		}

		void SpriteEditor::OnImGuiRender(bool& open)
		{
			if (open)
			{			
				if (ImGui::Begin("Sprite Editor", &open))
				{
					const ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
					m_ViewSectionWidth = viewportPanelSize.x - m_ToolSectionWidth - 5.0f;

					EditorHelper::DrawSplitter(false, 5.0f, &m_ToolSectionWidth, &m_ViewSectionWidth, 50.0f, 50.0f);
					
					
					tools();
					ImGui::SameLine();
					viewer();
				}
				ImGui::End();
			}
		}
		void SpriteEditor::OnEvent(Event& event)
		{		
			if (m_ViewportHovered && m_ViewportFocused)
				m_Camera.OnEvent(event);
		}
		void SpriteEditor::SetContext(const Ref<Texture2D>& context)
		{
			if (m_PixelBuffer)
			{
				delete[]m_PixelBuffer;
				m_PixelBuffer = nullptr;
			}
			m_Context = context;
			
			m_SpriteRenderer.SubTexture = Ref<SubTexture>::Create(m_Context);
			m_Output = Ref<SubTexture>::Create(m_Context);

			const float aspect = (float)m_Context->GetWidth() / (float)m_Context->GetHeight();
			m_Transform.Scale.x = 1.0f * aspect;
			m_Transform.WorldTransform = m_Transform.GetTransform();
			memset(m_OutputPath, 0, _MAX_PATH);
		}
		void SpriteEditor::handlePanelResize(const glm::vec2& newSize)
		{
			if (m_ViewportSize.x != newSize.x || m_ViewportSize.y != newSize.y)
			{
				m_ViewportSize = newSize;
				m_RenderPass->GetSpecification().TargetFramebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y, true);
				m_Camera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			}
		}
		void SpriteEditor::handleDragAndDrop()
		{
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM", 0))
				{
					const char* assetPath = (char*)payload->Data;
					const std::string strPath(assetPath);
					
					if (AssetManager::GetAssetTypeFromExtension(Utils::GetExtension(strPath)) == AssetType::Texture)
					{
						SetContext(AssetManager::GetAsset<Texture2D>(AssetManager::GetAssetHandle(strPath)));
					}
				}
				ImGui::EndDragDropTarget();
			}
		}
		void SpriteEditor::viewer()
		{
			if (ImGui::BeginChild("##Viewer", ImVec2(m_ViewSectionWidth, 0)))
			{			
				ImVec2 viewerPanelSize = ImGui::GetContentRegionAvail();
				m_ViewportFocused = ImGui::IsWindowFocused();
				m_ViewportHovered = ImGui::IsWindowHovered();
				ImGuiLayer* imguiLayer = Application::Get().GetImGuiLayer();
				const bool blocked = imguiLayer->GetBlockedEvents();
				if (blocked)
					imguiLayer->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);


				if (m_Context.Raw() && !ImGui::IsWindowCollapsed())
				{
					handlePanelResize({ viewerPanelSize.x, viewerPanelSize.y });
					//const uint32_t renderID = m_RenderPass->GetSpecification().TargetFramebuffer->GetColorAttachmentRendererID(0);
					//ImGui::Image(reinterpret_cast<void*>((void*)(uint64_t)renderID), viewerPanelSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
				}
			}
			ImGui::EndChild();
			handleDragAndDrop();
		}
		void SpriteEditor::tools()
		{
			if (ImGui::BeginChild("##Sprite", ImVec2(m_ToolSectionWidth, 0)))
			{
				bool enabled = true;
				EditorHelper::DrawNodeControl("Sprite", m_Output, [&](auto& value) {

					glm::vec4 border = CalcBorders(value);
					const char* names[4] = {
						 "L", "B", "R", "T"
					};
					EditorHelper::DrawVec4Control2x2("Border", names, border, 0.0f, 65.0f);
					SetBorder(value, border);

					EditorHelper::BeginColumns("Name", 2, 65.0f);
					const float spacing = 5.0f;
					const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
					ImGui::PushItemWidth(ImGui::CalcItemWidth() + lineHeight * 2.0f - spacing);
					ImGui::InputText("##Name", m_OutputPath, _MAX_PATH);
					ImGui::PopItemWidth();
					EditorHelper::EndColumns();

					if (ImGui::Button("Save"))
					{
						const std::string name = Utils::GetFilenameWithoutExtension(m_OutputPath);
						if (!name.empty())
						{
							Application& app = Application::Get();
							std::string folderPath = FileSystem::OpenFolder(
								app.GetWindow().GetNativeWindow(),
								app.GetApplicationDir()
							);
							if (std::filesystem::is_directory(folderPath))
							{
								std::replace(folderPath.begin(), folderPath.end(), '\\', '/');
								folderPath = folderPath.substr(app.GetApplicationDir().size() + 1, folderPath.size() - app.GetApplicationDir().size());

								if (!FileSystem::Exists(folderPath + "/" + name + ".subtex"))
								{
									AssetManager::CreateAsset<SubTexture>(name + ".subtex", folderPath, m_Context, m_Output->GetTexCoords());
								}
								else
								{
									XYZ_CORE_WARN("Asset with the same name already exists in the directory");
								}
							}
						}
						else
						{
							XYZ_CORE_WARN("Asset can not have an empty name");
						}
					}
				}, enabled);
			}
			ImGui::EndChild();
		}
	}
}


