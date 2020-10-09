#include "stdafx.h"
#include "SpriteEditorPanel.h"


namespace XYZ {

	static glm::vec4 SelectionToTexCoords(const glm::vec2& contextSize, const glm::vec2& contextPos, const glm::vec4& selection)
	{
		glm::vec4 select = { selection.x - contextPos.x, selection.y - contextPos.y, selection.z - contextPos.x, selection.w - contextPos.y };
		glm::vec4 result = {
			select.x / contextSize.x,  select.y / contextSize.y,
			select.z / contextSize.x,  select.w / contextSize.y
		};
		return result;
	}

	static glm::vec2 MouseToWorld(const glm::vec2& point, const glm::vec2& windowSize)
	{
		glm::vec2 offset = { windowSize.x / 2,windowSize.y / 2 };
		return { point.x - offset.x, offset.y - point.y };
	}

	static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
	{
		return (pos.x + size.x > point.x &&
			pos.x		   < point.x&&
			pos.y + size.y >  point.y &&
			pos.y < point.y);
	}

	SpriteEditorPanel::SpriteEditorPanel()
	{
		m_ZoomLevel = 1.0f;
		m_SelectedSelection = sc_InvalidSelection;
		m_ContextSize = glm::vec2(0.0f);
		m_NewSelection = glm::vec4(0.0f);
		InGui::Begin(m_SpriteEditorID, "Sprite Editor", { -200,-200 }, { 300,300 });
		InGui::End();
		m_Window = InGui::GetWindow(m_SpriteEditorID);
		m_Window->Flags &= ~InGuiWindowFlag::AutoPosition;
		m_Window->Flags &= ~InGuiWindowFlag::EventListener;
		m_Window->Flags |= InGuiWindowFlag::MenuEnabled;
		m_Window->OnResizeCallback = Hook(&SpriteEditorPanel::onInGuiWindowResize, this);
		m_BackgroundTexture = Texture2D::Create(TextureWrap::Repeat, "Assets/Textures/checkerboard.png");
	}
	void SpriteEditorPanel::SetContext(const Ref<Texture2D>& context)
	{
		m_Context = context;
		m_ContextSize = { (float)m_Context->GetWidth(),(float)m_Context->GetHeight() };		
		m_ContextSize += glm::vec2(m_Window->Size.x - m_ContextSize.x, m_Window->Size.y - m_ContextSize.y);
	}
	bool SpriteEditorPanel::OnInGuiRender()
	{
		bool active = InGui::Begin(m_SpriteEditorID, "Sprite Editor", { -200,-200 }, { 300,300 });
		
		InGui::Image("Background", m_BackgroundTexture->GetRendererID(), m_Window->Position, m_Window->Size,  { 0,0,1,1 }, 1.0f);
		if (active)
		{		
			if (m_Context)
			{
				glm::vec2 size = m_ContextSize;
				glm::vec2 position = m_Window->Position + (m_Window->Size / 2.0f) - (size / 2.0f);
				InGui::Image("Context", m_Context->GetRendererID(), position, size);
			}

			for (auto& selection : m_Selections)
			{
				glm::vec2 size = { (selection.z - selection.x) , (selection.w - selection.y) };
				glm::vec2 pos = { selection.x , selection.y };
				InGui::Selection(pos, size, m_SelectionsColor);
			}
			if (m_SelectedSelection != sc_InvalidSelection)
			{
				auto& selection = m_Selections[m_SelectedSelection];
				glm::vec2 size = { (selection.z - selection.x) , (selection.w - selection.y) };
				glm::vec2 pos = { selection.x , selection.y };
				InGui::Selection(pos, size, { 1,0,0,1 });
			}

			if (InGui::MenuBar("Selection", 90.0f, m_MenuOpen))
			{
				if (InGui::MenuItem("Auto Selection", { 150,25 }))
				{
					m_MenuOpen = false;
				}
				else if (InGui::MenuItem("Reset Selections", { 150,25 }))
				{
					m_Selections.clear();
					m_Sprites.clear();
					m_SelectedSelection = sc_InvalidSelection;
					m_MenuOpen = false;
				}
			}
			m_NewSelection = InGui::Selector(m_Selecting);
		}

		InGui::End();
		return active;
	}
	void SpriteEditorPanel::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&SpriteEditorPanel::onMouseButtonRelease, this));
		dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&SpriteEditorPanel::onMouseButtonPress, this));
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
				
				glm::vec2 size = m_ContextSize;
				glm::vec2 position = m_Window->Position + (m_Window->Size / 2.0f) - (size / 2.0f);
				glm::vec4 texCoords = SelectionToTexCoords(m_ContextSize, position, m_NewSelection);

				m_Sprites.push_back(Ref<SubTexture2D>::Create(m_Context, texCoords));
				m_Selections.push_back(m_NewSelection);		
			}
		}
		return false;
	}
	bool SpriteEditorPanel::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		m_SelectedSelection = sc_InvalidSelection;
		if (event.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_RIGHT))
		{
			auto [mx, my] = Input::GetMousePosition();
			auto [width, height] = Input::GetWindowSize();
			glm::vec2 mousePos = MouseToWorld({ mx,my }, { width,height });
			
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
		}
		return false;
	}
	void SpriteEditorPanel::onInGuiWindowResize(const glm::vec2& size)
	{
		if (m_SelectedSelection != sc_InvalidSelection)
		{
			// TODO: calculate new size of selections
			m_Selections[m_SelectedSelection].x += size.x - m_ContextSize.x;
		}
		m_ContextSize += glm::vec2(size.x - m_ContextSize.x, size.y - m_ContextSize.y);
	}
}