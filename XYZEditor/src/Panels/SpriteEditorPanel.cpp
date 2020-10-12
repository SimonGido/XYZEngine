#include "stdafx.h"
#include "SpriteEditorPanel.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace XYZ {

	static glm::vec4 TexCoordsFromSelection(const glm::vec4& selection,const glm::vec2& contextPos, const glm::vec2 contextScale)
	{
		glm::vec2 selectionSize = { selection.z - selection.x, selection.w - selection.y };
		glm::vec2 offsetLeft =  { selection.x + fabs(contextPos.x), selection.y + fabs(contextPos.y) };
		glm::vec2 offsetRight = { selection.z + fabs(contextPos.x), selection.w + fabs(contextPos.y) };
		offsetLeft.x /= contextScale.x;
		offsetRight.x /= contextScale.x;

		return { 
				 offsetLeft.x, 
				 offsetLeft.y,
				 offsetRight.x,
				 offsetRight.y 
		};
	}

	static glm::vec2 GetWorldPositionFromInGui(const InGuiWindow& window, const EditorCamera& camera)
	{
		auto [x, y] = Input::GetMousePosition();
		auto [width, height] = Input::GetWindowSize();
		x -= ((float)width / 2.0f) - fabs(window.Position.x);
		y -= ((float)height / 2.0f) - window.Position.y - window.Size.y;

		float cameraBoundWidth = (camera.GetAspectRatio() * camera.GetZoomLevel()) * 2;
		float cameraBoundHeight = camera.GetZoomLevel() * 2;
		auto pos = camera.GetPosition();

		x = (x / window.Size.x) * cameraBoundWidth - cameraBoundWidth * 0.5f;
		y = cameraBoundHeight * 0.5f - (y / window.Size.y) * cameraBoundHeight;

		return { x + pos.x , y + pos.y };
	}


	static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
	{
		return (pos.x + size.x > point.x &&
			pos.x		   < point.x&&
			pos.y + size.y >  point.y &&
			pos.y < point.y);
	}

	SpriteEditorPanel::SpriteEditorPanel(AssetManager& assetManager)
		:
		m_AssetManager(assetManager)
	{
		m_FBO = FrameBuffer::Create({ 300,300 });
		m_FBO->CreateColorAttachment(FrameBufferFormat::RGBA16F);
		m_FBO->CreateDepthAttachment();
		m_FBO->Resize();

		m_BackgroundTexture = Texture2D::Create(TextureWrap::Repeat, "Assets/Textures/checkerboard.png");
		m_Material = Material::Create(Shader::Create("Assets/Shaders/DefaultShader.glsl"));
		m_Material->Set("u_Texture", m_BackgroundTexture);
		m_Material->Set("u_Color", glm::vec4(1, 1, 1, 1));

		m_SelectedSelection = sc_InvalidSelection;
		m_NewSelection = glm::vec4(0.0f);
		m_ContextScale = glm::vec4(0.0f);
		m_ContextPos = glm::vec4(0.0f);

		InGui::RenderWindow(m_SpriteEditorID, "Sprite Editor", m_FBO->GetColorAttachment(0).RendererID, { -200,-200 }, { 300,300 });
		InGui::End();
		
		m_Window = InGui::GetWindow(m_SpriteEditorID);
		//m_Window->Flags &= ~InGuiWindowFlag::AutoPosition;
		m_Window->Flags &= ~InGuiWindowFlag::EventListener;
		m_Window->Flags |= InGuiWindowFlag::MenuEnabled;
		m_Window->OnResizeCallback = Hook(&SpriteEditorPanel::onInGuiWindowResize, this);
		
		auto [width, height] = Input::GetWindowSize();
		m_FBO->SetSpecification({ (uint32_t)width,(uint32_t)height });
		m_FBO->Resize();
		m_Transform = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		m_Camera.OnResize(m_Window->Size);
		m_Camera.SetCameraMouseMoveSpeed(0.008f);
	}
	void SpriteEditorPanel::SetContext(const Ref<Texture2D>& context)
	{
		m_Context = context;
		m_Material->Set("u_Texture", m_Context, CONTEXT);
		
		float scale = (float)context->GetWidth() / (float)context->GetHeight();
		m_ContextScale = { scale, 1.0f };
		m_ContextPos = { -scale / 2.0f, -0.5f };
		m_Transform = glm::scale(glm::mat4(1.0f), glm::vec3(scale, 1.0f, 1.0f));
	}
	bool SpriteEditorPanel::OnInGuiRender(Timestep ts)
	{
		keepCameraOnContext();
		onRender(ts);
		m_ActiveWindow = false;
		if (InGui::RenderWindow(m_SpriteEditorID, "Sprite Editor", m_FBO->GetColorAttachment(0).RendererID, { -200,-200 }, { 300,300 }))
		{
			m_ActiveWindow = true;
			if (m_Selecting)
			{
				glm::vec2 relativeMousePos = GetWorldPositionFromInGui(*m_Window, m_Camera);
				m_NewSelection.z = relativeMousePos.x;
				m_NewSelection.w = relativeMousePos.y;
			}
			
		}
		if (InGui::MenuBar("File", 90.0f, m_ExportOpen))
		{
			m_SelectionOpen = false;
			if (InGui::MenuItem("Export All", { 150.0f, 25.0f }))
			{
				if (!m_Sprites.empty())
				{
					auto& app = Application::Get();
					std::string filepath = app.OpenFolder();
					if (!filepath.empty())
					{
						uint32_t counter = 1;
						for (auto sprite : m_Sprites)
						{
							m_AssetManager.RegisterAsset(filepath + "\\Subtexture" + std::to_string(counter++) + ".subtex", sprite);
						}
					}
				}
			}
		}
		if (InGui::MenuBar("Selection", 90.0f, m_SelectionOpen))
		{
			m_ExportOpen = false;
			if (InGui::MenuItem("Auto Selection", { 150.0f, 25.0f }))
			{
				m_SelectionOpen = false;
			}
			else if (InGui::MenuItem("Reset Selections", { 150.0f, 25.0f }))
			{
				m_Selections.clear();
				m_Sprites.clear();
				m_SelectedSelection = sc_InvalidSelection;
				m_SelectionOpen = false;
			}
		}
		InGui::End();
		return m_ActiveWindow;
	}
	
	void SpriteEditorPanel::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>(Hook(&SpriteEditorPanel::onWindowResize, this));
		if (m_ActiveWindow)
		{
			dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&SpriteEditorPanel::onMouseButtonRelease, this));
			dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&SpriteEditorPanel::onMouseButtonPress, this));
			m_Camera.OnEvent(event);
		}
	}
	Ref<SubTexture2D> SpriteEditorPanel::GetSelectedSprite() const
	{
		if (m_SelectedSelection != sc_InvalidSelection)
		{
			return m_Sprites[m_SelectedSelection];
		}
		return nullptr;	
	}
	bool SpriteEditorPanel::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		if (event.IsButtonReleased(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
		{
			if (m_Selecting)
			{
				// Flip
				if (m_NewSelection.x > m_NewSelection.z)
				{
					float tmp = m_NewSelection.x;
					m_NewSelection.x = m_NewSelection.z;
					m_NewSelection.z = tmp;
				}
				if (m_NewSelection.y > m_NewSelection.w)
				{
					float tmp = m_NewSelection.y;
					m_NewSelection.y = m_NewSelection.w;
					m_NewSelection.w = tmp;
				}

				m_Sprites.push_back(Ref<SubTexture2D>::Create(m_Context, TexCoordsFromSelection(m_NewSelection, m_ContextPos, m_ContextScale)));
				m_Selections.push_back(m_NewSelection);
			}
			m_Selecting = false;
		}
		return false;
	}
	bool SpriteEditorPanel::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		m_SelectedSelection = sc_InvalidSelection;
		if (event.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
		{
			if (!m_SelectionOpen && !m_ExportOpen)
			{
				glm::vec2 relativeMousePos = GetWorldPositionFromInGui(*m_Window, m_Camera);
				m_NewSelection.x = relativeMousePos.x;
				m_NewSelection.y = relativeMousePos.y;

				m_Selecting = true;
				m_SelectedSelection = sc_InvalidSelection;
			}
		}
		else if (event.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_RIGHT))
		{
			glm::vec2 mousePos = GetWorldPositionFromInGui(*m_Window, m_Camera);
			uint32_t counter = 0;
			for (auto& selection : m_Selections)
			{	
				glm::vec2 size = { (selection.z - selection.x) , (selection.w - selection.y) };
				glm::vec2 pos = { selection.x , selection.y };
				if (Collide(pos, size, mousePos))
				{
					m_SelectedSelection = counter;
					break;
				}
				counter++;
			}
			m_Selecting = false;
		}
		return false;
	}
	bool SpriteEditorPanel::onWindowResize(WindowResizeEvent& event)
	{
		m_FBO->SetSpecification({ (uint32_t)(event.GetWidth()), (uint32_t)(event.GetHeight()) });
		m_FBO->Resize();
		m_Camera.OnResize(m_Window->Size);
		return false;
	}
	void SpriteEditorPanel::onInGuiWindowResize(const glm::vec2& size)
	{
		m_Camera.OnResize(size);
	}
	void SpriteEditorPanel::submitSelection(const glm::vec4& selection, const glm::vec4& color)
	{
		Renderer2D::SubmitLine({ selection.x, selection.y,0.0f }, { selection.z, selection.y,0.0f }, color);
		Renderer2D::SubmitLine({ selection.z, selection.y,0.0f }, { selection.z, selection.w,0.0f }, color);
		Renderer2D::SubmitLine({ selection.z, selection.w,0.0f }, { selection.x, selection.w,0.0f }, color);
		Renderer2D::SubmitLine({ selection.x, selection.w,0.0f }, { selection.x, selection.y,0.0f }, color);
	}
	void SpriteEditorPanel::onRender(Timestep ts)
	{
		m_Camera.OnUpdate(ts);
		glm::vec2 winSize = { Input::GetWindowSize().first, Input::GetWindowSize().second };
		m_FBO->Bind();
		RenderCommand::SetClearColor(glm::vec4(0.1, 0.1, 0.1, 1));
		RenderCommand::Clear();
		Renderer2D::BeginScene({ m_Camera.GetViewProjectionMatrix(), winSize });
		Renderer2D::SetMaterial(m_Material);
		Renderer2D::SubmitQuad(m_Transform, { 0.0f,0.0f,m_ContextScale.x, m_ContextScale.y }, BACKGROUND, { 1.0f, 1.0f, 1.0f, 1.0f });
		Renderer2D::SubmitQuad(m_Transform, { 0.0f,0.0f,1.0f,1.0f }, CONTEXT, { 1.0f, 1.0f, 1.0f, 1.0f });
		for (auto& selection : m_Selections)
			submitSelection(selection, { 1,1,1,1 });
		if (m_SelectedSelection != sc_InvalidSelection)
			submitSelection(m_Selections[m_SelectedSelection], m_SelectionsColor);
		if (m_Selecting)
			submitSelection(m_NewSelection, m_NewSelectionColor);

		Renderer2D::Flush();
		Renderer2D::FlushLines();
		Renderer2D::EndScene();
		m_FBO->Unbind();
		
	}
	void SpriteEditorPanel::keepCameraOnContext()
	{
		float xBorder = m_ContextScale.x / 2.0f;
		float yBorder = m_ContextScale.y / 2.0f;
		if (m_Camera.GetPosition().x < -xBorder)
		{
			float difX = (-xBorder) - m_Camera.GetPosition().x;
			m_Camera.Translate({ difX,0,0 });
		}
		else if (m_Camera.GetPosition().x > xBorder)
		{
			float difX = xBorder - m_Camera.GetPosition().x;
			m_Camera.Translate({ difX,0,0 });
		}

		if (m_Camera.GetPosition().y < -yBorder)
		{
			float dify = (-yBorder) - m_Camera.GetPosition().y;
			m_Camera.Translate({ 0,dify,0 });
		}
		else if (m_Camera.GetPosition().y > yBorder)
		{
			float dify = yBorder - m_Camera.GetPosition().y;
			m_Camera.Translate({ 0,dify,0 });
		}

	}
}