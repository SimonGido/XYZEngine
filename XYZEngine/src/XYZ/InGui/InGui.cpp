#include "stdafx.h"
#include "InGui.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"
#include "InGuiFactory.h"

#include <ini.h>

namespace XYZ {
	InGuiContext* InGui::s_Context = nullptr;


	static enum Color
	{
		RedToGreen,
		GreenToBlue,
		BlueToRed,
		None
	};
	static glm::vec4 ColorFrom6SegmentColorRectangle(const glm::vec2& position, const glm::vec2& size, const glm::vec2& mousePos)
	{
		static constexpr uint32_t numColorSegments = 3;

		float segmentSize = size.x / numColorSegments;
		float pos = mousePos.x - position.x;
		uint32_t segment = (uint32_t)floor(pos / segmentSize);
		float distance = (pos - (segment * segmentSize)) / segmentSize;


		switch (segment)
		{
		case RedToGreen:
			return { 1.0f - distance, distance,0.0f,1.0f };
		case GreenToBlue:
			return { 0.0f, 1.0f - distance, distance, 1.0f };
		case BlueToRed:
			return { distance,0.0f,1.0f - distance,1.0f };
		case None:
			return { distance,0.0f,1.0f - distance,1.0f };
		}

		return { 0,1,0,1 };
	}
	static glm::vec4 CalculatePixelColor(const glm::vec4& pallete, const glm::vec2& position, const glm::vec2& size, const InGuiPerFrameData& frameData)
	{
		glm::vec2 pos = frameData.MousePosition - position;
		glm::vec2 scale = pos / size;

		glm::vec4 diff = pallete - glm::vec4{ 1, 1, 1, 1 };

		glm::vec4 result = { scale.y,scale.y, scale.y,1 };
		result += glm::vec4(scale.y * scale.x * diff.x, scale.y * scale.x * diff.y, scale.y * scale.x * diff.z, 0);

		return result;
	}

	static void MoveVertices(InGuiVertex* vertices, const glm::vec2& position, size_t offset, size_t count)
	{
		for (size_t i = offset; i < count + offset; ++i)
		{
			vertices[i].Position.x += position.x;
			vertices[i].Position.y += position.y;
		}
	}
	static void HandleInputText(std::string& text, int code, int mode ,bool capslock)
	{
		size_t currentSize = text.size();
		switch (code)
		{
		case ToUnderlying(KeyCode::XYZ_KEY_BACKSPACE):
			if (!text.empty())
				text.pop_back();
			return;
		case ToUnderlying(KeyCode::XYZ_KEY_0):
			text.push_back('0');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_1):
			text.push_back('1');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_2):
			text.push_back('2');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_3):
			text.push_back('3');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_4):
			text.push_back('4');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_5):
			text.push_back('5');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_6):
			text.push_back('6');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_7):
			text.push_back('7');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_8):
			text.push_back('8');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_9):
			text.push_back('9');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_PERIOD):
			text.push_back('.');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_A):
			text.push_back('a');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_B):
			text.push_back('b');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_C):
			text.push_back('c');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_D):
			text.push_back('d');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_E):
			text.push_back('e');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_F):
			text.push_back('f');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_G):
			text.push_back('g');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_H):
			text.push_back('h');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_I):
			text.push_back('i');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_J):
			text.push_back('j');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_K):
			text.push_back('k');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_L):
			text.push_back('l');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_M):
			text.push_back('m');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_O):
			text.push_back('o');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_P):
			text.push_back('p');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_Q):
			text.push_back('q');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_R):
			text.push_back('r');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_S):
			text.push_back('s');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_T):
			text.push_back('t');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_U):
			text.push_back('u');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_V):
			text.push_back('v');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_W):
			text.push_back('w');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_X):
			text.push_back('x');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_Y):
			text.push_back('y');
			break;
		case ToUnderlying(KeyCode::XYZ_KEY_Z):
			text.push_back('z');
			break;
		}

		// Shift / Capslock
		if ((mode == ToUnderlying(KeyMode::XYZ_MOD_SHIFT) || capslock) && currentSize < text.size())
		{
			text[currentSize] = std::toupper(text[currentSize]);
		}
	}


	static glm::vec2 HandleWindowSpacing(const glm::vec2& uiSize, InGuiPerFrameData& frameData)
	{
		// Set position to the position of current window
		auto window = frameData.CurrentWindow;
		glm::vec2 position = window->Position;
		glm::vec2 offset = { 10, 10 };

		// Find the highest widget in row
		if (uiSize.y > frameData.MaxHeightInRow)
			frameData.MaxHeightInRow = uiSize.y;


		// If widget position is going to be outside of the window
		if (frameData.CurrentWindow->Size.x <= frameData.WindowSpaceOffset.x + uiSize.x)
		{
			// Set window offset x to zero
			frameData.WindowSpaceOffset.x = 0.0f;

			// Subtract the highest widget in row and offset y from window offset y 
			frameData.WindowSpaceOffset.y -= frameData.MaxHeightInRow + offset.y;

			frameData.MaxHeightInRow = uiSize.y;
		}

		// Subtract from position widget size y
		position.y -= uiSize.y;
		// Add to position window space offset and offset
		position += frameData.WindowSpaceOffset + glm::vec2{ offset.x,-offset.y };


		frameData.WindowSpaceOffset.x += uiSize.x + offset.x;

		return position;
	}
	static glm::vec2 StringToVec2(const std::string& src)
	{
		glm::vec2 val;
		size_t split = src.find(",", 0);

		val.x = std::stof(src.substr(0, split));
		val.y = std::stof(src.substr(split + 1, src.size() - split));

		return val;
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

	void InGui::Init(const InGuiRenderConfiguration& renderConfig)
	{
		s_Context = new InGuiContext;
		s_Context->RenderConfiguration = renderConfig;
		s_Context->PerFrameData.WindowSize.x = Input::GetWindowSize().first;
		s_Context->PerFrameData.WindowSize.y = Input::GetWindowSize().second;
		loadDockSpace();
	}
	void InGui::Destroy()
	{
		saveDockSpace();
		delete s_Context;
	}
	void InGui::BeginFrame()
	{
		auto [mx,my] = Input::GetMousePosition();
		s_Context->PerFrameData.MousePosition = MouseToWorld({ mx,my }, s_Context->PerFrameData.WindowSize);
		InGuiRenderer::BeginScene({ s_Context->PerFrameData.WindowSize });
		s_Context->DockSpace->Begin();
	}
	void InGui::EndFrame()
	{
		if (s_Context->PerFrameData.ModifiedWindow)
		{
			resolveResize(*s_Context->PerFrameData.ModifiedWindow);
			resolveMove(*s_Context->PerFrameData.ModifiedWindow);
		}

		
		std::sort(s_Context->RenderQueue.begin(), s_Context->RenderQueue.end(), [](const InGuiMesh* a, const InGuiMesh* b) {
			return a->Material->GetSortKey() < b->Material->GetSortKey();
		});

		InGuiRenderer::BeginScene({ s_Context->PerFrameData.WindowSize });
		for (auto mesh : s_Context->RenderQueue)
		{
			InGuiRenderer::SetMaterial(mesh->Material);
			InGuiRenderer::SubmitUI(*mesh);
		}
		for (auto window : s_Context->Windows)
		{
			InGuiRenderer::SubmitLineMesh(window.second->LineMesh);
		}
		s_Context->DockSpace->End(s_Context->PerFrameData.MousePosition, *s_Context);
		InGuiRenderer::Flush();
		InGuiRenderer::FlushLines();
		InGuiRenderer::EndScene();

		s_Context->RenderConfiguration.NumTexturesInUse = InGuiRenderConfiguration::DefaultTextureCount;
		size_t numMeshes = s_Context->RenderQueue.size();
		s_Context->RenderQueue.clear();
		s_Context->RenderQueue.reserve(numMeshes);

		// Clean codes
		s_Context->PerFrameData.KeyCode = ToUnderlying(KeyCode::XYZ_KEY_NONE);
		s_Context->PerFrameData.Mode = ToUnderlying(KeyMode::XYZ_MOD_NONE);
		s_Context->PerFrameData.Code = ToUnderlying(MouseCode::XYZ_MOUSE_NONE);
	}
	bool InGui::Begin(const std::string& name, const glm::vec2& position, const glm::vec2& size, const InGuiRenderConfiguration& renderConfig)
	{
		XYZ_ASSERT(!s_Context->PerFrameData.CurrentWindow, "Missing end call");
		std::string copyName = name;
		std::transform(copyName.begin(), copyName.end(), copyName.begin(), ::tolower);

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiWindow* window = getWindow(copyName);
		if (!window)
			window = createWindow(name, position, size);

		frameData.CurrentWindow = window;
		if (!(window->Flags & Visible))
			return false;

		frameData.WindowSpaceOffset.y = window->Size.y;
		window->Mesh.Material = renderConfig.Material;

		// Check if window is hoovered
		glm::vec2 winSize = window->Size + glm::vec2(InGuiWindow::PanelSize, InGuiWindow::PanelSize);
		if (Collide(window->Position, winSize, frameData.MousePosition))
		{
			window->Flags |= Modified;
			window->Flags |= Hoovered;
			if (window->Flags & EventListener)
				s_Context->PerFrameData.EventReceivingWindow = window;
		}
		// Check if was modified
		if (window->Flags & Modified)
			InGuiFactory::GenerateWindow(*window, renderConfig);
		
		// Push to render queue
		s_Context->RenderQueue.push_back(&window->Mesh);


		return !(window->Flags & Collapsed);
	}
	void InGui::End()
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");
		
		s_Context->PerFrameData.CurrentWindow->Flags &= ~Modified;
		s_Context->PerFrameData.CurrentWindow->Flags &= ~Hoovered;
		s_Context->PerFrameData.ResetWindowData();
	}

	bool InGui::BeginPopup(const std::string& name, const glm::vec2& size, bool& open, const InGuiRenderConfiguration& renderConfig)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiWindow* window = frameData.CurrentWindow;
	
		if (window->Flags & Modified)
		{
			glm::vec4 color = renderConfig.DefaultColor;
			glm::vec2 position = HandleWindowSpacing(size, frameData);
			if (Collide(position, size, frameData.MousePosition))
			{
				color = renderConfig.HooverColor;
				if (!(frameData.Flags & ClickHandled))
				{
					if (frameData.Flags & LeftMouseButtonPressed)
					{
						frameData.Flags |= ClickHandled;
						open = !open;
					}
				}
			}
			InGuiFactory::GenerateButton(position, size, color, name, frameData, renderConfig);
			frameData.PopupOffset.x = position.x;
			frameData.PopupOffset.y = position.y - size.y;
		}

		return open;
	}

	bool InGui::PopupItem(const std::string& name, const glm::vec2& size, const InGuiRenderConfiguration& renderConfig)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiWindow* window = frameData.CurrentWindow;
		bool pressed = false;
		if (window->Flags & Modified)
		{
			glm::vec4 color = renderConfig.DefaultColor;
			glm::vec2 position = { frameData.PopupOffset.x, frameData.PopupOffset.y };
			if (Collide(position, size, frameData.MousePosition))
			{
				color = renderConfig.HooverColor;
				if (!(frameData.Flags & ClickHandled))
				{
					if (frameData.Flags & LeftMouseButtonPressed)
					{
						frameData.Flags |= ClickHandled;
						pressed = true;
					}
				}
			}
			InGuiFactory::GenerateButton(position, size, color, name, frameData, renderConfig);
		}
		frameData.PopupOffset.y -= size.y;
		return pressed;
	}

	void InGui::EndPopup()
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");
		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		frameData.PopupOffset = { 0,0 };
	}


	bool InGui::MenuBar(const std::string& name, float width, bool& open, const InGuiRenderConfiguration& renderConfig)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiWindow* window = frameData.CurrentWindow;
		if (window->Flags & Modified)
		{
			if (window->Flags & MenuEnabled)
			{
				glm::vec4 color = renderConfig.DefaultColor;
				glm::vec2 size = { width,InGuiWindow::PanelSize };
				glm::vec2 position = { window->Position.x + frameData.MenuBarOffset.x, window->Position.y + window->Size.y };
				if (Collide(position, size, frameData.MousePosition))
				{
					color = renderConfig.HooverColor;
					if (!(frameData.Flags & ClickHandled))
					{
						if (frameData.Flags & LeftMouseButtonPressed)
						{
							frameData.Flags |= ClickHandled;
							open = !open;
						}
					}
				}
				InGuiFactory::GenerateMenuBar(position, size, color, name, frameData, renderConfig);

				frameData.MenuBarOffset.x += width;
				frameData.MenuBarOffset.y = position.y - InGuiWindow::PanelSize;
				frameData.MenuItemOffset = position.x;
			}
		}
		return open;
	}

	bool InGui::MenuItem(const std::string& name, const glm::vec2& size, const InGuiRenderConfiguration& renderConfig)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiWindow* window = frameData.CurrentWindow;
		bool pressed = false;
		if (window->Flags & Modified)
		{
			if (window->Flags & MenuEnabled)
			{
				glm::vec4 color = renderConfig.DefaultColor;
				glm::vec2 position = { frameData.MenuItemOffset,frameData.MenuBarOffset.y };
				if (Collide(position, size, frameData.MousePosition))
				{
					color = renderConfig.HooverColor;
					if (!(frameData.Flags & ClickHandled))
					{
						if (frameData.Flags & LeftMouseButtonPressed)
						{
							frameData.Flags |= ClickHandled;
							pressed = true;
						}
					}
				}
				InGuiFactory::GenerateMenuBar(position, size, color, name, frameData, renderConfig);
				frameData.MenuBarOffset.y -= size.y;
			}
		}
		return pressed;
	}

	bool InGui::BeginGroup(const std::string& name, bool& open, const InGuiRenderConfiguration& renderConfig)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiWindow* window = frameData.CurrentWindow;
		
		if (window->Flags & Modified)
		{
			glm::vec2 winPos = window->Position;
			glm::vec2 winSize = window->Size;
			glm::vec2 panelPos = { winPos.x, HandleWindowSpacing({winSize.x - 5,InGuiWindow::PanelSize},frameData).y };
			glm::vec2 minButtonPos = { panelPos.x + 5, panelPos.y };
			glm::vec4 color = renderConfig.DefaultColor;
			if (Collide(minButtonPos, { InGuiWindow::PanelSize,InGuiWindow::PanelSize }, frameData.MousePosition))
			{
				color = renderConfig.HooverColor;
				if ((frameData.Flags & LeftMouseButtonPressed)
					&& !(frameData.Flags & ClickHandled))
				{
					open = !open;
					frameData.Flags |= ClickHandled;
				}
			}
			InGuiFactory::GenerateGroup(panelPos, color, name, open, frameData, renderConfig);
		}

		return open;
	}

	void InGui::EndGroup()
	{
	}

	bool InGui::Button(const std::string& name, const glm::vec2& size, const InGuiRenderConfiguration& renderConfig)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiWindow* window = frameData.CurrentWindow;
		bool pressed = false;
		if (window->Flags & Modified)
		{
			glm::vec4 color = renderConfig.DefaultColor;
			glm::vec2 position = HandleWindowSpacing(size, frameData);
			if (Collide(position, size, frameData.MousePosition))
			{
				color = renderConfig.HooverColor;
				if (!(frameData.Flags & ClickHandled))
				{
					if ((frameData.Flags & LeftMouseButtonPressed))
					{
						frameData.Flags |= ClickHandled;
						pressed = true;
					}
				}
			}
			InGuiFactory::GenerateButton(position, size, color, name, frameData, renderConfig);
		}

		return pressed;
	}

	bool InGui::Checkbox(const std::string& name, const glm::vec2& size, bool& value, const InGuiRenderConfiguration& renderConfig)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiWindow* window = frameData.CurrentWindow;

		if (window->Flags & Modified)
		{
			glm::vec4 color = renderConfig.DefaultColor;

			size_t offset = window->Mesh.Vertices.size();
			auto [width, height] = InGuiFactory::GenerateText({ 0.7f,0.7f }, color, name, frameData, renderConfig);
			glm::vec2 position = HandleWindowSpacing({ size.x + width + 5,size.y }, frameData);
			glm::vec2 textOffset = { size.x + 5,(size.y / 2) - (height / 2) };
			MoveVertices(window->Mesh.Vertices.data(), position + textOffset, offset, name.size() * 4);

			if (Collide(position, size, frameData.MousePosition))
			{
				color = renderConfig.HooverColor;
				if (!(frameData.Flags & ClickHandled))
				{
					if (frameData.Flags & LeftMouseButtonPressed)
					{
						frameData.Flags |= ClickHandled;
						value = !value;
					}
				}
			}
			InGuiFactory::GenerateCheckbox(position, size, color, name, value, frameData, renderConfig);
		}

		return value;
	}

	bool InGui::Slider(const std::string& name, const glm::vec2& size, float& value, float valueScale, const InGuiRenderConfiguration& renderConfig)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiWindow* window = frameData.CurrentWindow;
		bool modified = false;
		if (window->Flags & Modified)
		{
			glm::vec4 color = renderConfig.DefaultColor;
			glm::vec2 position = HandleWindowSpacing({ size.x,size.y * 2 }, frameData);
			if (Collide(position, size, frameData.MousePosition))
			{
				color = renderConfig.HooverColor;
				modified = (frameData.Flags & LeftMouseButtonPressed);
				if (modified)
				{
					float start = position.x;
					value = (frameData.MousePosition.x - start) / valueScale;
				}
			}
			InGuiFactory::GenerateSlider(position, size, color, name,value * valueScale, frameData, renderConfig);
		}

		return modified;
	}

	bool InGui::Image(const std::string& name, uint32_t rendererID, const glm::vec2& size, const InGuiRenderConfiguration& renderConfig)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiWindow* window = frameData.CurrentWindow;
		bool pressed = false;
		if (window->Flags & Modified)
		{
			glm::vec4 color = renderConfig.DefaultColor;
			glm::vec2 position = HandleWindowSpacing(size, frameData);
			if (Collide(position, size, frameData.MousePosition))
			{
				color = renderConfig.HooverColor;
				if (!(frameData.Flags & ClickHandled))
				{
					pressed = (frameData.Flags & LeftMouseButtonPressed);
					if (pressed)
						frameData.Flags |= ClickHandled;
				}
			}
			InGuiFactory::GenerateImage(position, size, color, rendererID, frameData, renderConfig);
		}

		return pressed;
	}

	bool InGui::TextArea(const std::string& name, std::string& text, const glm::vec2& size, bool& modified, const InGuiRenderConfiguration& renderConfig)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiWindow* window = frameData.CurrentWindow;

		if (window->Flags & Modified)
		{
			glm::vec4 color = renderConfig.DefaultColor;		
			glm::vec2 position = HandleWindowSpacing(size, frameData);
		
			if (Collide(position, size, frameData.MousePosition))
			{		
				if (!(frameData.Flags & ClickHandled))
				{
					if (frameData.Flags & LeftMouseButtonPressed)
					{
						frameData.Flags |= ClickHandled;
						modified = !modified;
					}			
				}
			}
			if (modified)
			{
				color = renderConfig.HooverColor;
				HandleInputText(text, frameData.KeyCode, frameData.Mode, frameData.CapslockEnabled);
			}
			InGuiFactory::GenerateTextArea(position, size, color, name, text, frameData, renderConfig);
		}
		
		return modified;
	}

	bool InGui::Text(const std::string& text, const glm::vec2& scale, const glm::vec4& color, const InGuiRenderConfiguration& renderConfig)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiWindow* window = frameData.CurrentWindow;

		if (window->Flags & Modified)
		{
			size_t offset = window->Mesh.Vertices.size();
			auto [width, height] = InGuiFactory::GenerateText(scale, color, text, frameData, renderConfig);
			glm::vec2 size = { width,height };
			glm::vec2 position = HandleWindowSpacing(size, frameData);
			MoveVertices(window->Mesh.Vertices.data(), position, offset, text.size() * 4);
			if (Collide(position, size, frameData.MousePosition))
			{
				if (!(frameData.Flags & ClickHandled))
				{
					
					if (frameData.Flags & LeftMouseButtonPressed)
					{
						frameData.Flags |= ClickHandled;
						return true;
					}
				}
			}
		}

		return false;
	}

	bool InGui::ColorPicker4(const std::string& name, const glm::vec2& size, glm::vec4& pallete, glm::vec4& color, const InGuiRenderConfiguration& renderConfig)
	{
		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiWindow* window = frameData.CurrentWindow;

		if (window->Flags & Modified)
		{
			glm::vec2 position = HandleWindowSpacing(size, frameData);

			bool modified = false;

			Separator();
			if (ColorPallete4("", { size.x, 25.0f }, pallete))
			{
				modified = true;
			}
			InGuiFactory::GenerateColorPicker4(position, size, pallete, frameData, renderConfig);
			Separator();
			if (Slider("R: " + std::to_string(color.x), { size.x,15 }, color.x, size.x))
				modified = true;
			Separator();
			if (Slider("G: " + std::to_string(color.y), { size.x,15 }, color.y, size.x))
				modified = true;
			Separator();
			if (Slider("B: " + std::to_string(color.z), { size.x,15 }, color.z, size.x))
				modified = true;
			Separator();

			if (Collide(position, size, frameData.MousePosition))
			{
				if (!(frameData.Flags & ClickHandled))
				{
					if (frameData.Flags & LeftMouseButtonPressed)
					{
						frameData.Flags |= ClickHandled;

						modified = true;
						color = CalculatePixelColor(pallete, position, size, frameData);
					}
				}
			}
			return modified;
		}
		return false;
	}

	bool InGui::ColorPallete4(const std::string& name, const glm::vec2& size, glm::vec4& color, const InGuiRenderConfiguration& renderConfig)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiWindow* window = frameData.CurrentWindow;
		if (window->Flags & Modified)
		{
			glm::vec2 position = HandleWindowSpacing(size, frameData);
			InGuiFactory::Generate6SegmentColorRectangle(position, size, frameData, renderConfig);
			if (Collide(position, size, frameData.MousePosition))
			{
				if (frameData.Flags & LeftMouseButtonPressed)
				{
					color = ColorFrom6SegmentColorRectangle(position, size, frameData.MousePosition);
					return true;
				}
			}
		}
		return false;
	}

	bool InGui::RenderWindow(const std::string& name, uint32_t rendererID, const glm::vec2& position, const glm::vec2& size, float panelSize, InGuiRenderConfiguration& renderConfig)
	{
		XYZ_ASSERT(!s_Context->PerFrameData.CurrentWindow, "Missing end call");
		std::string copyName = name;
		std::transform(copyName.begin(), copyName.end(), copyName.begin(), ::tolower);

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiWindow* window = getWindow(copyName);
		if (!window)
			window = createWindow(name, position, size);
		
		frameData.CurrentWindow = window;
		if (!(window->Flags & Visible))
			return false;

		window->Mesh.Material = renderConfig.Material;

		// Check if window is hoovered
		glm::vec2 winSize = window->Size + glm::vec2(InGuiWindow::PanelSize, InGuiWindow::PanelSize);
		if (Collide(window->Position, winSize, frameData.MousePosition))
		{
			window->Flags |= Modified;
			window->Flags |= Hoovered;
			if (window->Flags & EventListener)
				s_Context->PerFrameData.EventReceivingWindow = window;
		}
		// Check if was modified
		if (window->Flags & Modified)
			InGuiFactory::GenerateRenderWindow(*window, rendererID, renderConfig);

		// Push to render queue
		s_Context->RenderQueue.push_back(&window->Mesh);


		return (window->Flags & Hoovered);
	}

	void InGui::Separator()
	{
		s_Context->PerFrameData.WindowSpaceOffset.x = s_Context->PerFrameData.CurrentWindow->Size.x;
	}

	glm::vec4 InGui::Selector()
	{
		return glm::vec4();
	}
	
	bool InGui::OnWindowResize(const glm::vec2& winSize)
	{
		s_Context->PerFrameData.WindowSize = winSize;
		s_Context->DockSpace->OnWindowResize(winSize);
		return false;
	}

	bool InGui::OnLeftMouseButtonPress()
	{
		s_Context->DockSpace->OnLeftMouseButtonPress();
		s_Context->PerFrameData.Code = ToUnderlying(MouseCode::XYZ_MOUSE_BUTTON_LEFT);
		s_Context->PerFrameData.Flags |= LeftMouseButtonPressed;
		s_Context->PerFrameData.Flags &= ~ClickHandled;
		InGuiWindow* window = s_Context->PerFrameData.EventReceivingWindow;
		if (window)
		{
			glm::vec2 size = window->Size + glm::vec2(InGuiWindow::PanelSize, InGuiWindow::PanelSize);
			if (Collide(window->Position, size, s_Context->PerFrameData.MousePosition))
			{
				if (detectCollapse(*window))
				{}
		
				return true;
			}
		}	
		return false;
	}

	bool InGui::OnRightMouseButtonPress()
	{
		s_Context->PerFrameData.Code = ToUnderlying(MouseCode::XYZ_MOUSE_BUTTON_RIGHT);
		s_Context->PerFrameData.Flags |= RightMouseButtonPressed;
		s_Context->PerFrameData.Flags &= ~ClickHandled;
		if (s_Context->DockSpace->OnRightMouseButtonPress(s_Context->PerFrameData.MousePosition))
			return true;
		InGuiWindow* window = s_Context->PerFrameData.EventReceivingWindow;
		if (window)
		{
			glm::vec2 size = window->Size + glm::vec2(0, InGuiWindow::PanelSize);
			if (Collide(window->Position, size, s_Context->PerFrameData.MousePosition))
			{
				if (detectResize(*window))
				{}
				else if(detectMoved(*window))
				{}
				

				s_Context->PerFrameData.ModifiedWindow = window;
				
				return true;
			}
		}		
		return false;
	}

	bool InGui::OnLeftMouseButtonRelease()
	{
		s_Context->PerFrameData.Flags &= ~LeftMouseButtonPressed;
		s_Context->PerFrameData.Code = ToUnderlying(MouseCode::XYZ_MOUSE_NONE);
		if (s_Context->PerFrameData.ModifiedWindow)
		{
			s_Context->PerFrameData.ModifiedWindow->Flags &= ~(Moved | LeftResizing | RightResizing | TopResizing | BottomResizing);
			s_Context->PerFrameData.ModifiedWindow = nullptr;
		}
		
		return false;
	}

	bool InGui::OnRightMouseButtonRelease()
	{
		s_Context->PerFrameData.Code = ToUnderlying(MouseCode::XYZ_MOUSE_NONE);
		s_Context->DockSpace->OnRightMouseButtonRelease(s_Context->PerFrameData.ModifiedWindow, s_Context->PerFrameData.MousePosition);
		s_Context->PerFrameData.Flags &= ~RightMouseButtonPressed;
		if (s_Context->PerFrameData.ModifiedWindow)
		{
			s_Context->PerFrameData.ModifiedWindow->Flags &= ~(Moved | LeftResizing | RightResizing | TopResizing | BottomResizing);
			s_Context->PerFrameData.ModifiedWindow = nullptr;
			auto& app = Application::Get();
			app.GetWindow().SetCursor(XYZ_ARROW_CURSOR);
		}
		
		return false;
	}

	bool InGui::OnKeyPress(int key, int mod)
	{
		if (mod == ToUnderlying(KeyMode::XYZ_MOD_CAPS_LOCK))
			s_Context->PerFrameData.CapslockEnabled = !s_Context->PerFrameData.CapslockEnabled;

		s_Context->PerFrameData.KeyCode = key;
		s_Context->PerFrameData.Mode = mod;
		return false;
	}

	bool InGui::OnKeyRelease()
	{
		s_Context->PerFrameData.KeyCode = ToUnderlying(KeyCode::XYZ_KEY_NONE);
		s_Context->PerFrameData.Mode = ToUnderlying(KeyMode::XYZ_MOD_NONE);
		return false;
	}

	InGuiWindow* InGui::getWindow(const std::string& name)
	{
		auto it = s_Context->Windows.find(name);
		if (it != s_Context->Windows.end())
			return it->second;
		return nullptr;
	}
	InGuiWindow* InGui::createWindow(const std::string& name, const glm::vec2& position, const glm::vec2& size)
	{
		std::string copyName = name;
		std::transform(copyName.begin(), copyName.end(), copyName.begin(), ::tolower);

		InGuiWindow* window = new InGuiWindow;
		window->Position = position;
		window->Size = size;
		window->Name = name;
		window->Flags |= Modified;
		window->Flags |= EventListener;
		window->Flags |= Visible;
		s_Context->Windows.insert({ copyName,window });
		return window;
	}
	bool InGui::detectResize(InGuiWindow& window)
	{
		if (!(window.Flags & Docked) && !(window.Flags & Moved))
		{
			auto& app = Application::Get();
			auto& frameData = s_Context->PerFrameData;
			glm::vec2 offset = { 10,10 };
			auto& mousePos = frameData.MousePosition;
			bool resized = false;

			// Right side
			if (mousePos.x >= window.Position.x + window.Size.x - offset.x)
			{
				window.Flags |= RightResizing;
				app.GetWindow().SetCursor(XYZ_HRESIZE_CURSOR);
				resized = true;
			}
			// Left side
			else if (mousePos.x <= window.Position.x + offset.x)
			{
				window.Flags |= LeftResizing;
				app.GetWindow().SetCursor(XYZ_HRESIZE_CURSOR);
				resized = true;
			}
			// Bottom side
			if (mousePos.y <= window.Position.y + offset.y)
			{
				window.Flags |= BottomResizing;
				app.GetWindow().SetCursor(XYZ_VRESIZE_CURSOR);
				resized = true;
			}
			// Top side
			else if (mousePos.y >= window.Position.y + window.Size.y - offset.y + InGuiWindow::PanelSize)
			{
				window.Flags |= TopResizing;
				app.GetWindow().SetCursor(XYZ_VRESIZE_CURSOR);
				resized = true;
			}
			return resized;
		}
		return false;
	}
	bool InGui::detectMoved(InGuiWindow& window)
	{
		if (s_Context->PerFrameData.MousePosition.y >= window.Position.y + window.Size.y
			&& !(window.Flags & Moved))
		{
			window.Flags |= Moved;
			window.Flags |= Modified;
			s_Context->PerFrameData.ModifiedWindowMouseOffset = s_Context->PerFrameData.MousePosition - window.Position - glm::vec2{ 0, window.Size.y };
			return true;
		}
		return false;
	}
	bool InGui::detectCollapse(InGuiWindow& window)
	{
		glm::vec2 minButtonPos = {
				window.Position.x + window.Size.x - InGuiWindow::PanelSize,
				window.Position.y + window.Size.y
		};
		glm::vec2 minButtonSize = { InGuiWindow::PanelSize,InGuiWindow::PanelSize };

		if (Collide(minButtonPos, minButtonSize, s_Context->PerFrameData.MousePosition))
		{
			window.Flags ^= Collapsed;
			window.Flags |= Modified;
			return true;
		}
		return false;
	}
	void InGui::resolveResize(InGuiWindow& window)
	{
		auto& mousePos = s_Context->PerFrameData.MousePosition;
		auto& frameData = s_Context->PerFrameData;
		if (!(window.Flags & Docked))
		{
			window.Flags |= Modified;
			if (window.Size.x > window.MinimalWidth)
			{
				if (window.Flags & RightResizing)
				{
					window.Size.x = mousePos.x - window.Position.x;
				}
				else if (window.Flags & LeftResizing)
				{
					window.Size.x = window.Position.x + window.Size.x - mousePos.x;
					window.Position.x = mousePos.x;
				}
			}
			else
			{
				window.Size.x += InGuiWindow::PanelSize;
				window.Flags &= ~LeftResizing;
				window.Flags &= ~RightResizing;
			}

			if (window.Flags & BottomResizing)
			{
				window.Size.y = window.Position.y + window.Size.y - mousePos.y;
				window.Position.y = mousePos.y;
			}
			else if (window.Flags & TopResizing)
			{
				window.Size.y = mousePos.y - window.Position.y - InGuiWindow::PanelSize;
			}
		}
	}
	void InGui::resolveMove(InGuiWindow& window)
	{
		if (window.Flags & Moved)
		{
			auto& frameData = s_Context->PerFrameData;
			glm::vec2 pos = frameData.MousePosition - frameData.ModifiedWindowMouseOffset;
			window.Position = { pos.x, pos.y - window.Size.y };
			s_Context->DockSpace->RemoveWindow(&window);
			s_Context->DockSpace->m_DockSpaceVisible = true;
			window.Flags |= Modified;
		}
	}


	void InGui::loadDockSpace()
	{
		static constexpr uint32_t numPropertiesDockNode = 5;
		mINI::INIFile file("ingui.ini");
		mINI::INIStructure ini;

		auto& frameData = s_Context->PerFrameData;
		auto& windows = s_Context->Windows;

		frameData.WindowSize.x = (float)Input::GetWindowSize().first;
		frameData.WindowSize.y = (float)Input::GetWindowSize().second;

		if (file.read(ini))
		{
			std::unordered_map<uint32_t, InGuiWindow*> windowMap;

			// Load windows
			auto it = ini.begin();
			while (it->first != "dockspace" && it != ini.end())
			{
				windows[it->first] = new InGuiWindow();
				windows[it->first]->Name = it->first;
				windows[it->first]->Position = StringToVec2(it->second.get("position"));
				windows[it->first]->Size = StringToVec2(it->second.get("size"));
				int32_t id = atoi(it->second.get("docknode").c_str());
				if (id != -1)
					windowMap[id] = windows[it->first];

				if ((bool)atoi(it->second.get("collapsed").c_str()))
					windows[it->first]->Flags |= Collapsed;

				windows[it->first]->Flags |= Modified;
				windows[it->first]->Flags |= EventListener;
				windows[it->first]->Flags |= Visible;

				it++;
			}


			std::unordered_map<uint32_t, InGuiDockNode*> dockMap;
			std::unordered_map<uint32_t, int32_t> parentMap;
			uint32_t id = 0;

			// Load dockspace
			auto el = it->second.begin();
			while (el != it->second.end())
			{
				std::string nodeID = std::to_string(id);
				glm::vec2 pos = StringToVec2(it->second.get("node position " + nodeID));
				glm::vec2 size = StringToVec2(it->second.get("node size " + nodeID));
				int32_t parentID = atoi(it->second.get("node parent " + nodeID).c_str());
				if (parentID != -1)
					parentMap[id] = parentID;

				dockMap[id] = new InGuiDockNode(pos, size, id);
				dockMap[id]->Split = (SplitAxis)atoi(it->second.get("node split " + nodeID).c_str());
				dockMap[id]->Dock = (DockPosition)atoi(it->second.get("node dockposition " + nodeID).c_str());

				id++;
				el += numPropertiesDockNode;
			}

			// Setup tree
			for (auto id : parentMap)
			{
				dockMap[id.first]->Parent = dockMap[id.second];
				if (dockMap[id.first]->Dock == DockPosition::Left || dockMap[id.first]->Dock == DockPosition::Bottom)
					dockMap[id.first]->Parent->Children[0] = dockMap[id.first];
				else if (dockMap[id.first]->Dock != DockPosition::None)
					dockMap[id.first]->Parent->Children[1] = dockMap[id.first];
			}
			// Setup windows
			for (auto win : windowMap)
			{
				win.second->Flags |= Docked;
				win.second->DockNode = dockMap[win.first];
				dockMap[win.first]->Windows.push_back(win.second);
			}

			// Setup new dockspace and root
			s_Context->DockSpace = new InGuiDockSpace(dockMap[0]);
			s_Context->DockSpace->m_NodeCount = dockMap.size();
		}
		else
		{
			InGuiDockNode* root = new InGuiDockNode({ -frameData.WindowSize.x / 2,-frameData.WindowSize.y / 2 }, frameData.WindowSize, 0);
			s_Context->DockSpace = new InGuiDockSpace(root);
		}
	}

	static void SaveDockSpace(mINI::INIStructure& ini, const InGuiDockNode* node)
	{
		std::string nodeID = std::to_string(node->ID);
		ini["Dockspace"]["node position " + nodeID] = std::to_string(node->Position.x) + "," + std::to_string(node->Position.y);
		ini["Dockspace"]["node size " + nodeID] = std::to_string(node->Size.x) + "," + std::to_string(node->Size.y);
		ini["Dockspace"]["node split " + nodeID] = std::to_string(ToUnderlying<SplitAxis>(node->Split));
		ini["Dockspace"]["node dockposition " + nodeID] = std::to_string(ToUnderlying<DockPosition>(node->Dock));

		if (node->Parent)
			ini["Dockspace"]["node parent " + nodeID] = std::to_string(node->Parent->ID);
		else
			ini["Dockspace"]["node parent " + nodeID] = std::to_string(-1);


		if (node->Children[0])
			SaveDockSpace(ini, node->Children[0]);

		if (node->Children[1])
			SaveDockSpace(ini, node->Children[1]);
	}

	void InGui::saveDockSpace()
	{
		mINI::INIFile file("ingui.ini");
		mINI::INIStructure ini;
		file.generate(ini);

		for (auto& it : s_Context->Windows)
		{
			std::string pos = std::to_string(it.second->Position.x) + "," + std::to_string(it.second->Position.y);
			std::string size = std::to_string(it.second->Size.x) + "," + std::to_string(it.second->Size.y);
			ini[it.first]["position"] = pos;
			ini[it.first]["size"] = size;
			bool collapsed = (it.second->Flags & Collapsed);
			ini[it.first]["collapsed"] = std::to_string(collapsed);
			if (it.second->DockNode)
				ini[it.first]["Docknode"] = std::to_string(it.second->DockNode->ID);
			else
				ini[it.first]["Docknode"] = std::to_string(-1);

			delete it.second;
		}
		SaveDockSpace(ini, s_Context->DockSpace->m_Root);
		file.write(ini);

		delete s_Context->DockSpace;
		s_Context->DockSpace = nullptr;
	}
}