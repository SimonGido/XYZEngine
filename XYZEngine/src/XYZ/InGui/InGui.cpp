#include "stdafx.h"
#include "InGui.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Renderer/RenderCommand.h"
#include "InGuiFactory.h"

#include <ini.h>


#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>



namespace XYZ {
	InGuiContext* InGui::s_Context = nullptr;


	static enum Color
	{
		RedToGreen,
		GreenToBlue,
		BlueToRed,
		None
	};



	static void SubmitTexture(uint32_t rendererID, std::vector<TextureRendererIDPair>& texturePairs, const InGuiRenderConfiguration& renderConfig)
	{
		uint32_t textureID = 0;
		for (auto& pair : texturePairs)
		{
			if (pair.RendererID == rendererID)
				textureID = pair.TextureID;
		}
		if (!textureID)
		{
			textureID = renderConfig.NumTexturesInUse;
			renderConfig.NumTexturesInUse++;
		}
		texturePairs.push_back({ textureID,rendererID });
	}
	static glm::vec2 HandlePinInputSpacing(const glm::vec2& uiSize, InGuiPerFrameData& frameData)
	{
		// Set position to the position of current window
		auto node = frameData.CurrentNode;
		
		glm::vec2 offset = { 0, 10 };
		frameData.LeftNodePinOffset += offset.y;
		glm::vec2 position = node->Position;
		position.y += node->Size.y;
		position.y -= frameData.LeftNodePinOffset;
		frameData.LeftNodePinOffset += uiSize.y;

		return position;
	}
	static glm::vec2 HandlePinOutputSpacing(const glm::vec2& uiSize, InGuiPerFrameData& frameData)
	{
		// Set position to the position of current window
		auto node = frameData.CurrentNode;

		glm::vec2 offset = { 0, 10 };
		frameData.RightNodePinOffset += offset.y;
		glm::vec2 position = node->Position;
		position.y += node->Size.y;
		position.y -= frameData.RightNodePinOffset;
		position.x += node->Size.x - uiSize.x;
		frameData.RightNodePinOffset += uiSize.y;
		return position;
	}
	static glm::vec2 ConvertToCamera(const glm::vec2& mousePos,const glm::vec2& winPos, const glm::vec2& winSize, const InGuiCamera& camera)
	{
		glm::vec2 offset = { winSize.x / 2,winSize.y / 2 };
		glm::vec2 pos = { mousePos.x - offset.x, mousePos.y - offset.y };

		pos.x += (winSize.x / 2 * camera.GetPosition().x) - winPos.x;
		pos.y += (winSize.y / 2 * camera.GetPosition().y) - winPos.y;
		return pos;

		return { mousePos.x + winPos.x + (winSize.x/2 * camera.GetPosition().x) , mousePos.y + winPos.y + (winSize.y/2 * camera.GetPosition().y) };
	}

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
		case ToUnderlying(KeyCode::XYZ_KEY_N):
			text.push_back('n');
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
		case ToUnderlying(KeyCode::XYZ_KEY_SPACE):
			text.push_back(' ');
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

	static glm::vec2 ResolveText(InGuiMesh& mesh, const char* text, const glm::vec2& size, const glm::vec4& color, const InGuiRenderConfiguration& renderConfig, InGuiPerFrameData& frameData)
	{
		size_t offset = mesh.Vertices.size();
		auto info = InGuiFactory::GenerateText({ 0.7f,0.7f }, color, text, mesh, renderConfig);
		glm::vec2 position = HandleWindowSpacing({ size.x + info.Size.x + 5,size.y * 2 }, frameData);
		glm::vec2 textOffset = { size.x + 5,(size.y / 2) - ((float)info.Size.y / 1.5f) };
		MoveVertices(mesh.Vertices.data(), position + textOffset, offset, info.Count * 4);

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

		for (auto window : s_Context->Windows)
		{
			delete window.second;
		}
		for (auto nodeWindow : s_Context->NodeWindows)
		{
			delete nodeWindow.second;
		}

		delete s_Context->DockSpace;
		delete s_Context;
	}
	void InGui::BeginFrame()
	{
		auto [mx,my] = Input::GetMousePosition();
		s_Context->PerFrameData.EventReceivingWindow = nullptr;
		s_Context->PerFrameData.MousePosition = MouseToWorld({ mx,my }, s_Context->PerFrameData.WindowSize);
		s_Context->DockSpace->Begin();
	}
	void InGui::EndFrame()
	{
		if (s_Context->PerFrameData.ModifiedWindow)
		{
			resolveResize(*s_Context->PerFrameData.ModifiedWindow);
			resolveMove(*s_Context->PerFrameData.ModifiedWindow);
		}

	
		InGuiRenderer::BeginScene({ glm::mat4(1.0f), s_Context->PerFrameData.WindowSize });
		InGuiRenderer::SetTexturePairs(s_Context->PerFrameData.TexturePairs);
		InGuiRenderer::SetMaterial(s_Context->RenderConfiguration.InMaterial);
		for (auto mesh : s_Context->RenderQueue.GetMeshes())
		{
			InGuiRenderer::SetMaterial(mesh->Material);
			InGuiRenderer::SubmitUI(*mesh);
		}
		for (auto mesh : s_Context->RenderQueue.GetLineMeshes())
		{
			InGuiRenderer::SubmitLineMesh(*mesh);
		}
		s_Context->DockSpace->End(s_Context->PerFrameData.MousePosition, s_Context->PerFrameData, s_Context->RenderConfiguration);
		InGuiRenderer::Flush();
		InGuiRenderer::FlushLines();
		InGuiRenderer::EndScene();

		s_Context->RenderConfiguration.NumTexturesInUse = InGuiRenderConfiguration::DefaultTextureCount;
		s_Context->PerFrameData.TexturePairs.clear();
	
		s_Context->RenderQueue.Reset();
		
		
		// Clean codes
		s_Context->PerFrameData.KeyCode = ToUnderlying(KeyCode::XYZ_KEY_NONE);
		s_Context->PerFrameData.Mode = ToUnderlying(KeyMode::XYZ_MOD_NONE);
		s_Context->PerFrameData.Code = ToUnderlying(MouseCode::XYZ_MOUSE_NONE);
	}
	bool InGui::Begin(uint32_t id, const char* name, const glm::vec2& position, const glm::vec2& size)
	{
		XYZ_ASSERT(!s_Context->PerFrameData.CurrentWindow, "Missing end call");
	
		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = GetWindow(id);
		if (!window)
			window = createWindow(id, position, size);

		window->Name = name;

		frameData.CurrentWindow = window;
		if (!(window->Flags & InGuiWindowFlag::Visible))
			return false;

		frameData.WindowSpaceOffset.y = window->Size.y;
		window->Mesh.Material = renderConfig.InMaterial;

		// Check if window is hoovered
		glm::vec2 winSize = window->Size + glm::vec2(0.0f, InGuiWindow::PanelSize);
		if (Collide(window->Position, winSize, frameData.MousePosition))
		{
			window->Flags |= InGuiWindowFlag::Modified;
			window->Flags |= InGuiWindowFlag::Hoovered;

			if (frameData.MousePosition.y < window->Position.y + window->Size.y)
				s_Context->RenderQueue.Push(&window->Mesh, &window->LineMesh);
			else
				s_Context->RenderQueue.PushOverlay(&window->Mesh, &window->LineMesh);
			if (window->Flags & InGuiWindowFlag::EventListener)
				frameData.EventReceivingWindow = window;
		}	
		else
			s_Context->RenderQueue.Push(&window->Mesh, &window->LineMesh);
		

		// Check if was modified
		if (window->Flags & InGuiWindowFlag::Modified)
			InGuiFactory::GenerateWindow(name, *window, renderConfig);

		return !(window->Flags & InGuiWindowFlag::Collapsed);
	}
	void InGui::End()
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");
		
		s_Context->PerFrameData.CurrentWindow->Flags &= ~InGuiWindowFlag::Modified;
		s_Context->PerFrameData.CurrentWindow->Flags &= ~InGuiWindowFlag::Hoovered;
		s_Context->PerFrameData.CurrentWindow->Flags &= ~InGuiWindowFlag::LeftClicked;
		s_Context->PerFrameData.CurrentWindow->Flags &= ~InGuiWindowFlag::RightClicked;
		s_Context->PerFrameData.ResetWindowData();
	}

	bool InGui::BeginPopup(const char* name,glm::vec2& position, const glm::vec2& size, bool& open)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;
	
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec4 color = renderConfig.DefaultColor;
			if (window->Flags & InGuiWindowFlag::AutoPosition)
				position = HandleWindowSpacing(size, frameData);
			if (Collide(position, size, frameData.MousePosition))
			{
				color = renderConfig.HooverColor;
				if (resolveLeftClick())
				{
					open = !open;
				}
			}
			
			InGuiFactory::GenerateButton(position, size, color, name, window->Mesh, renderConfig);
			frameData.PopupOffset.x = position.x;
			frameData.PopupOffset.y = position.y - size.y;
		}

		return open;
	}

	bool InGui::PopupItem(const char* name,const glm::vec2& size)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;
		bool pressed = false;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec4 color = renderConfig.DefaultColor;
			glm::vec2 position = { frameData.PopupOffset.x, frameData.PopupOffset.y };
			if (Collide(position, size, frameData.MousePosition))
			{
				color = renderConfig.HooverColor;
				if (resolveLeftClick())
				{
					pressed = true;
				}
			}
			InGuiFactory::GenerateButton(position, size, color, name, window->Mesh, renderConfig);
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


	bool InGui::MenuBar(const char* name, float width, bool& open)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			if (window->Flags & InGuiWindowFlag::MenuEnabled)
			{
				glm::vec4 color = renderConfig.DefaultColor;
				glm::vec2 size = { width,InGuiWindow::PanelSize };
				glm::vec2 position = { window->Position.x + frameData.MenuBarOffset.x, window->Position.y + window->Size.y };
				if (Collide(position, size, frameData.MousePosition))
				{
					color = renderConfig.HooverColor;
					if (resolveLeftClick())
					{
						open = !open;
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

	bool InGui::MenuItem(const char* name, const glm::vec2& size)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;
		bool pressed = false;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			if (window->Flags & InGuiWindowFlag::MenuEnabled)
			{
				glm::vec4 color = renderConfig.DefaultColor;
				glm::vec2 position = { frameData.MenuItemOffset,frameData.MenuBarOffset.y };
				if (Collide(position, size, frameData.MousePosition))
				{
					color = renderConfig.HooverColor;
					if (resolveLeftClick())
					{
						pressed = true;
					}
				}
				InGuiFactory::GenerateMenuBar(position, size, color, name, frameData, renderConfig);
				frameData.MenuBarOffset.y -= size.y;
			}
		}
		return pressed;
	}

	bool InGui::BeginGroup(const char* name, const glm::vec2& position, bool& open)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;
		
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec2 winPos = window->Position;
			glm::vec2 winSize = window->Size;
			glm::vec2 panelPos = position;
			if (window->Flags & InGuiWindowFlag::AutoPosition)
				panelPos = { winPos.x, HandleWindowSpacing({winSize.x - 5,InGuiWindow::PanelSize},frameData).y };
			glm::vec2 minButtonPos = { panelPos.x + 5, panelPos.y };
			glm::vec4 color = renderConfig.DefaultColor;
			if (Collide(minButtonPos, { InGuiWindow::PanelSize,InGuiWindow::PanelSize }, frameData.MousePosition))
			{
				color = renderConfig.HooverColor;
				if (resolveLeftClick())
				{
					open = !open;
				}
			}
			InGuiFactory::GenerateGroup(panelPos, color, name, open, frameData, renderConfig);
		}

		return open;
	}

	void InGui::EndGroup()
	{
	}

	bool InGui::Button(const char* name, const glm::vec2& size)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;
		bool pressed = false;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec4 color = renderConfig.DefaultColor;
			glm::vec2 position = HandleWindowSpacing(size, frameData);
			if (Collide(position, size, frameData.MousePosition))
			{
				color = renderConfig.HooverColor;
				if (resolveLeftClick())
				{
					pressed = true;
				}
			}
			InGuiFactory::GenerateButton(position, size, color, name, window->Mesh, renderConfig);
		}

		return pressed;
	}

	bool InGui::Checkbox(const char* name, const glm::vec2& size, bool& value)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;

		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec4 color = renderConfig.DefaultColor;

			glm::vec2 position = ResolveText(window->Mesh, name, size, color, renderConfig, frameData);

			if (Collide(position, size, frameData.MousePosition))
			{
				color = renderConfig.HooverColor;
				if (resolveLeftClick())
				{		
					value = !value;
				}
			}
			InGuiFactory::GenerateCheckbox(position, size, color, name, value, window->Mesh, renderConfig);
		}

		return value;
	}

	bool InGui::Slider(const char* name, const glm::vec2& size, float& value, float valueScale)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;
		bool modified = false;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec4 color = renderConfig.DefaultColor;
			
			glm::vec2 position = ResolveText(window->Mesh, name, size, color, renderConfig, frameData);

			if (Collide(position, size, frameData.MousePosition))
			{
				color = renderConfig.HooverColor;
				modified = resolveLeftClick(false);
				if (modified)
				{
					float start = position.x;
					value = (frameData.MousePosition.x - start) / valueScale;
				}
			}
			InGuiFactory::GenerateSlider(position, size, color, name,value * valueScale,frameData.WindowSpaceOffset, window->Mesh, renderConfig);
		}

		return modified;
	}

	bool InGui::Image(const char* name, uint32_t rendererID, const glm::vec2& size, const glm::vec2& position, const glm::vec4& texCoords, float tilingFactor)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;
		bool pressed = false;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec2 pos = position;
			if (window->Flags & InGuiWindowFlag::AutoPosition)
				pos = HandleWindowSpacing(size, frameData);
			if (Collide(pos, size, frameData.MousePosition))
			{
				pressed = resolveLeftClick(false);
			}
			InGuiFactory::GenerateImage(pos, size, renderConfig.DefaultColor, texCoords, rendererID, window->Mesh, frameData.TexturePairs, renderConfig, tilingFactor);
		}
		else
		{
			SubmitTexture(rendererID, frameData.TexturePairs, renderConfig);
		}

		return pressed;
	}

	bool InGui::TextArea(const char* name, std::string& text, const glm::vec2& size, bool& modified)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;

		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec4 color = renderConfig.DefaultColor;

			glm::vec2 position = ResolveText(window->Mesh, name, size, color, renderConfig, frameData);
	
			if (Collide(position, size, frameData.MousePosition))
			{		
				if (resolveLeftClick())
				{
					modified = !modified;		
				}
			}
			else if (resolveLeftClick(false) || resolveRightClick(false))
			{
				modified = false;
			}

			if (modified)
			{
				color = renderConfig.HooverColor;
				HandleInputText(text, frameData.KeyCode, frameData.Mode, frameData.CapslockEnabled);
			}
			
					
			InGuiFactory::GenerateTextArea(position, size, color, name, text.c_str(), frameData.WindowSpaceOffset, window->Mesh, renderConfig);
		}
		
		return modified;
	}

	bool InGui::Text(const char* text, const glm::vec2& scale, const glm::vec4& color)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;

		if (window->Flags & InGuiWindowFlag::Modified)
		{
			size_t offset = window->Mesh.Vertices.size();
			auto info = InGuiFactory::GenerateText(scale, color, text, window->Mesh, renderConfig);
			glm::vec2 position = HandleWindowSpacing(info.Size, frameData);
			MoveVertices(window->Mesh.Vertices.data(), position, offset, info.Count * 4);
			if (Collide(position, info.Size, frameData.MousePosition))
			{
				return resolveLeftClick();
			}
		}

		return false;
	}

	bool InGui::ColorPicker4(const char* name, const glm::vec2& size, glm::vec4& pallete, glm::vec4& color)
	{
		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;

		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec2 position = HandleWindowSpacing(size, frameData);

			bool modified = false;

			Separator();
			if (ColorPallete4("", { size.x, 25.0f }, pallete))
			{
				modified = true;
			}
			InGuiFactory::GenerateColorPicker4(position, size, pallete, window->Mesh, renderConfig);
			Separator();

	
			if (Slider("R:", { size.x,15 }, color.x, size.x))
				modified = true;
			Separator();
			if (Slider("G:", { size.x,15 }, color.y, size.x))
				modified = true;
			Separator();
			if (Slider("B:", { size.x,15 }, color.z, size.x))
				modified = true;
			Separator();

			if (Collide(position, size, frameData.MousePosition))
			{
				if (resolveLeftClick())
				{			
					modified = true;
					color = CalculatePixelColor(pallete, position, size, frameData);
				}
			}
			return modified;
		}
		return false;
	}

	bool InGui::ColorPallete4(const char* name, const glm::vec2& size, glm::vec4& color)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec2 position = HandleWindowSpacing(size, frameData);
			InGuiFactory::Generate6SegmentColorRectangle(position, size, window->Mesh, renderConfig);
			if (Collide(position, size, frameData.MousePosition))
			{
				if (resolveLeftClick())
				{
					color = ColorFrom6SegmentColorRectangle(position, size, frameData.MousePosition);
					return true;
				}
			}
		}
		return false;
	}

	

	bool InGui::RenderWindow(uint32_t id,const char* name, uint32_t rendererID, const glm::vec2& position, const glm::vec2& size, float panelSize)
	{
		XYZ_ASSERT(!s_Context->PerFrameData.CurrentWindow, "Missing end call");


		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = GetWindow(id);
		if (!window)
			window = createWindow(id, position, size);
		
		window->Name = name;

		frameData.CurrentWindow = window;
		if (!(window->Flags & InGuiWindowFlag::Visible))
			return false;

		frameData.WindowSpaceOffset.y = window->Size.y;
		window->Mesh.Material = renderConfig.InMaterial;

		// Check if window is hoovered
		glm::vec2 winSize = window->Size + glm::vec2(0.0f, InGuiWindow::PanelSize);
		if (Collide(window->Position, winSize, frameData.MousePosition))
		{
			window->Flags |= InGuiWindowFlag::Modified;
			window->Flags |= InGuiWindowFlag::Hoovered;
			if (frameData.MousePosition.y < window->Position.y + window->Size.y)
				s_Context->RenderQueue.Push(&window->Mesh, &window->LineMesh);
			else
				s_Context->RenderQueue.PushOverlay(&window->Mesh, &window->LineMesh);


			if (window->Flags & InGuiWindowFlag::EventListener)
				frameData.EventReceivingWindow = window;
		}
		else
			s_Context->RenderQueue.Push(&window->Mesh, &window->LineMesh);
		
	
		if (window->Flags & InGuiWindowFlag::Modified)
			InGuiFactory::GenerateRenderWindow(name, *window, rendererID, frameData, renderConfig);
		else
			SubmitTexture(rendererID, frameData.TexturePairs, renderConfig);
		
		return (window->Flags & InGuiWindowFlag::Hoovered);
	}



	bool InGui::NodeWindow(uint32_t id, const char* name, const glm::vec2& position, const glm::vec2& size,float dt)
	{
		XYZ_ASSERT(!s_Context->PerFrameData.CurrentNodeWindow, "Missing end call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiNodeWindow* nodeWindow = GetNodeWindow(id);
		if (!nodeWindow)
			nodeWindow = createNodeWindow(id, position, size);

		frameData.CurrentNodeWindow = nodeWindow;
		nodeWindow->RenderWindow->Flags &= ~InGuiWindowFlag::AutoPosition;
		
		
		nodeWindow->FBO->Bind();	
		RenderCommand::SetClearColor(glm::vec4(0.1, 0.1, 0.1, 1));
		RenderCommand::Clear();
		
			
		bool result = RenderWindow(id, name, nodeWindow->FBO->GetColorAttachment(0).RendererID, position, size, 25.0f);
		if (result)
		{
			nodeWindow->RelativeMousePosition = ConvertToCamera(frameData.MousePosition,nodeWindow->RenderWindow->Position, frameData.WindowSize, nodeWindow->InCamera);
			nodeWindow->Mesh.Vertices.clear();
			nodeWindow->LineMesh.Vertices.clear();
			nodeWindow->InCamera.OnUpdate(dt);
			if (resolveLeftClick(false) || resolveRightClick(false))
			{
				if (nodeWindow->SelectedNode)
					nodeWindow->SelectedNode->Color = renderConfig.DefaultColor;
				nodeWindow->SelectedNode = nullptr;
			}
		}
	
		InGuiRenderer::BeginScene({ nodeWindow->InCamera.GetViewProjectionMatrix(), frameData.WindowSize });
		InGuiRenderer::SetMaterial(renderConfig.InMaterial);
			
		
		return result;
	}

	void InGui::PushConnection(uint32_t start, uint32_t end)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentNodeWindow, "Missing node window end call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiNodeWindow* nodeWindow = frameData.CurrentNodeWindow;
		InGuiWindow* window = frameData.CurrentWindow;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			auto startNode = nodeWindow->Nodes[start];
			auto endNode = nodeWindow->Nodes[end];
			glm::vec2 p0 = startNode->Position + (startNode->Size / 2.0f);
			glm::vec2 p1 = endNode->Position + (endNode->Size / 2.0f);
			InGuiFactory::GenerateArrowLine(nodeWindow->Mesh, nodeWindow->LineMesh, p0, p1, { 50.0f,50.0f }, s_Context->RenderConfiguration);
		}
	}

	bool InGui::BeginNode(uint32_t id, std::string& name, const glm::vec2& position, const glm::vec2& size, bool& modified)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentNodeWindow, "Missing node window end call");
		XYZ_ASSERT(!s_Context->PerFrameData.CurrentNode, "Missing node end call");

		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiNodeWindow* nodeWindow = frameData.CurrentNodeWindow;
		InGuiWindow* window = frameData.CurrentWindow;

		bool pressed = false;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			InGuiNode* node = getNode(id);
			if (!node)
				node = createNode(id, position, size);

			frameData.CurrentNode = node;
			glm::vec4 color = renderConfig.DefaultColor;
			
			if (resolveLeftClick(false))
			{
				if (Collide(node->Position, node->Size, nodeWindow->RelativeMousePosition))
				{
					nodeWindow->SelectedNode = node;
					node->Color = renderConfig.HooverColor;
					pressed = true;
					modified = false;
					
					frameData.Flags |= InGuiPerFameFlag::ClickHandled;
				}
				else if (Collide(node->Position + glm::vec2(0, node->Size.y), { node->Size.x, InGuiWindow::PanelSize }, nodeWindow->RelativeMousePosition))
				{
					modified = !modified;
					frameData.Flags |= InGuiPerFameFlag::ClickHandled;
				}
				else
				{
					modified = false;
				}
			}
			else if (resolveRightClick(false))
			{
				modified = false;
			}

			if (modified)
			{
				color = renderConfig.HooverColor;
				HandleInputText(name, frameData.KeyCode, frameData.Mode, frameData.CapslockEnabled);
			}

			InGuiFactory::GenerateNode(node->Position, node->Size, node->Color, color, name.c_str(), nodeWindow->Mesh, renderConfig);
		}
		return pressed;
	}

	bool InGui::BeginConnection(std::pair<uint32_t, uint32_t>& connection)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentNodeWindow, "Missing node window end call");
		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiNodeWindow* nodeWindow = frameData.CurrentNodeWindow;

		bool connected = false;
		if (nodeWindow->SelectedNode)
		{
			InGuiNode* selected = nodeWindow->SelectedNode;
					
			glm::vec2 p0 = selected->Position + (selected->Size / 2.0f);
			InGuiFactory::GenerateArrowLine(nodeWindow->Mesh, nodeWindow->LineMesh, p0, nodeWindow->RelativeMousePosition, { 50.0f,50.0f }, s_Context->RenderConfiguration);

			if (resolveLeftRelease(false))
			{
				for (auto node : nodeWindow->Nodes)
				{
					if (Collide(node.second->Position, node.second->Size, nodeWindow->RelativeMousePosition))
					{
						frameData.Flags |= InGuiPerFameFlag::ReleaseHandled;
						if (node.second->ID != selected->ID)
						{
							connection.first = selected->ID;
							connection.second = node.second->ID;
							connected = true;
							break;
						}
					}
				}		
				nodeWindow->SelectedNode->Color = s_Context->RenderConfiguration.DefaultColor;
				nodeWindow->SelectedNode = nullptr;
			}
			
			
		}
		return connected;
	}

	void InGui::EndNode()
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentNode, "Missint node begin call");
		s_Context->PerFrameData.LeftNodePinOffset =  InGuiWindow::PanelSize;
		s_Context->PerFrameData.RightNodePinOffset = InGuiWindow::PanelSize;
		s_Context->PerFrameData.CurrentNode = nullptr;
	}

	bool InGui::BeginInput(uint32_t id,const glm::vec2& size, const char* name)
	{
		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;

		InGuiNodeWindow* nodeWindow = frameData.CurrentNodeWindow;
		InGuiNode* node = frameData.CurrentNode;
		
		glm::vec2 position = HandlePinInputSpacing(size, frameData);

		glm::vec4 color = renderConfig.DefaultColor;
		if (Collide(position, size, nodeWindow->RelativeMousePosition))
		{
			color = renderConfig.HooverColor;
		}
		size_t offset = nodeWindow->Mesh.Vertices.size();
		auto [width, height] = InGuiFactory::GenerateText({ 0.7f,0.7f }, color, name, nodeWindow->Mesh, renderConfig);
		glm::vec2 textOffset = { size.x + 5,(size.y / 2) - (height / 2) };
		MoveVertices(nodeWindow->Mesh.Vertices.data(), position + textOffset, offset, strlen(name) * 4);
		InGuiFactory::GenerateQuad(position, size, color, nodeWindow->Mesh, renderConfig);
		return false;
	}

	void InGui::EndInput()
	{
	}

	bool InGui::BeginOutput(uint32_t id, const glm::vec2& size, const char* name)
	{
		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;

		InGuiNodeWindow* nodeWindow = frameData.CurrentNodeWindow;
		InGuiNode* node = frameData.CurrentNode;

		glm::vec2 position = HandlePinOutputSpacing(size, frameData);

		glm::vec4 color = renderConfig.DefaultColor;
		if (Collide(position, size, nodeWindow->RelativeMousePosition))
		{
			color = renderConfig.HooverColor;
			if (resolveLeftClick())
			{
				
			}
		}
		size_t offset = nodeWindow->Mesh.Vertices.size();
		auto info = InGuiFactory::GenerateText({ 0.7f,0.7f }, color, name, nodeWindow->Mesh, renderConfig);
		glm::vec2 textOffset = { -( 5 + info.Size.x), (size.y / 2) - (info.Size.y / 2) };
		MoveVertices(nodeWindow->Mesh.Vertices.data(), position + textOffset, offset, info.Count * 4);
		InGuiFactory::GenerateQuad(position, size, color, nodeWindow->Mesh, renderConfig);
		return false;
	}

	void InGui::EndOutput()
	{
	}

	void InGui::NodeWindowEnd()
	{
		End();
		XYZ_ASSERT(s_Context->PerFrameData.CurrentNodeWindow, "Missing begin call");
		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiNodeWindow* nodeWindow = frameData.CurrentNodeWindow;
		
		InGuiRenderer::SubmitUI(nodeWindow->Mesh);
		InGuiRenderer::SubmitLineMesh(nodeWindow->LineMesh);
		InGuiRenderer::FlushLines();
		InGuiRenderer::Flush();
	

		frameData.CurrentNodeWindow->FBO->Unbind();
		frameData.CurrentNodeWindow = nullptr;
	}


	void InGui::Separator()
	{
		s_Context->PerFrameData.WindowSpaceOffset.x = s_Context->PerFrameData.CurrentWindow->Size.x;
	}

	glm::vec4 InGui::Selector(bool& selecting)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");
		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;

		InGuiWindow* window = frameData.CurrentWindow;

		if (window->Flags & InGuiWindowFlag::Modified)
		{
			if (resolveLeftClick())
			{
				frameData.SelectedPoint = frameData.MousePosition;
				selecting = true;
			}
			if (selecting)
			{
				if (resolveLeftRelease(false) || (frameData.MousePosition.y >= window->Position.y + window->Size.y))
					selecting = false;

				glm::vec2 size = frameData.MousePosition - frameData.SelectedPoint;	
				InGuiFactory::GenerateFrame(window->LineMesh, frameData.SelectedPoint, size, renderConfig.LineColor);
				return glm::vec4(frameData.SelectedPoint, frameData.SelectedPoint + size);
			}
		}
		return glm::vec4(0);
	}

	void InGui::Selection(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		XYZ_ASSERT(s_Context->PerFrameData.CurrentWindow, "Missing begin call");
		InGuiPerFrameData& frameData = s_Context->PerFrameData;
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;

		if (window->Flags & InGuiWindowFlag::Modified)
		{
			InGuiFactory::GenerateFrame(window->LineMesh, position, size, color);
		}
	}
	
	bool InGui::OnWindowResize(const glm::vec2& winSize)
	{
		s_Context->PerFrameData.WindowSize = winSize;
		s_Context->RenderConfiguration.InMaterial->Set("u_ViewportSize", s_Context->PerFrameData.WindowSize);
		s_Context->DockSpace->OnWindowResize(winSize);
		return false;
	}

	bool InGui::OnLeftMouseButtonPress()
	{
		if (s_Context->DockSpace->OnLeftMouseButtonPress())
			return true;
		s_Context->PerFrameData.Code = ToUnderlying(MouseCode::XYZ_MOUSE_BUTTON_LEFT);
		s_Context->PerFrameData.Flags |= InGuiPerFameFlag::LeftMouseButtonPressed;
		s_Context->PerFrameData.Flags &= ~InGuiPerFameFlag::ClickHandled;
		s_Context->PerFrameData.Flags &= ~InGuiPerFameFlag::LeftMouseButtonReleased;

		InGuiWindow* window = s_Context->PerFrameData.EventReceivingWindow;
		if (window
			&& (window->Flags & InGuiWindowFlag::Visible))
		{		
			if (detectCollapse(*window))
			{
				return true;
			}
			if (!(window->Flags & InGuiWindowFlag::Collapsed))
			{
				window->Flags |= InGuiWindowFlag::LeftClicked;		
			}
			return true;
		}	
		return false;
	}

	bool InGui::OnRightMouseButtonPress()
	{
		if (s_Context->DockSpace->OnRightMouseButtonPress(s_Context->PerFrameData.MousePosition))
			return true;
		s_Context->PerFrameData.Code = ToUnderlying(MouseCode::XYZ_MOUSE_BUTTON_RIGHT);
		s_Context->PerFrameData.Flags |=  InGuiPerFameFlag::RightMouseButtonPressed;
		s_Context->PerFrameData.Flags &= ~InGuiPerFameFlag::ClickHandled;
		s_Context->PerFrameData.Flags &= ~InGuiPerFameFlag::RightMouseButtonReleased;
		
		InGuiWindow* window = s_Context->PerFrameData.EventReceivingWindow;
		if (window && (window->Flags & InGuiWindowFlag::Visible))
		{		
			if (detectResize(*window))
			{
				s_Context->PerFrameData.Flags |= InGuiPerFameFlag::ClickHandled;
				s_Context->PerFrameData.ModifiedWindow = window;
			}
			else if(detectMoved(*window))
			{
				s_Context->PerFrameData.Flags |= InGuiPerFameFlag::ClickHandled;
				s_Context->PerFrameData.ModifiedWindow = window;
			}		
			else
			{
				window->Flags |= InGuiWindowFlag::RightClicked;
			}
			return true;		
		}		
		
		return false;
	}

	bool InGui::OnLeftMouseButtonRelease()
	{
		s_Context->PerFrameData.Flags |=  InGuiPerFameFlag::LeftMouseButtonReleased;
		s_Context->PerFrameData.Flags &= ~InGuiPerFameFlag::ReleaseHandled;
		s_Context->PerFrameData.Flags &= ~InGuiPerFameFlag::LeftMouseButtonPressed;
		s_Context->PerFrameData.Code = ToUnderlying(MouseCode::XYZ_MOUSE_NONE);
		if (s_Context->PerFrameData.ModifiedWindow)
		{
			s_Context->PerFrameData.ModifiedWindow->Flags &= ~(InGuiWindowFlag::Moved | InGuiWindowFlag::LeftResizing | InGuiWindowFlag::RightResizing | InGuiWindowFlag::TopResizing | InGuiWindowFlag::BottomResizing);
			s_Context->PerFrameData.ModifiedWindow = nullptr;
		}
	
		return false;
	}

	bool InGui::OnRightMouseButtonRelease()
	{
		s_Context->PerFrameData.Code = ToUnderlying(MouseCode::XYZ_MOUSE_NONE);
		s_Context->DockSpace->OnRightMouseButtonRelease(s_Context->PerFrameData.ModifiedWindow, s_Context->PerFrameData.MousePosition);
		s_Context->PerFrameData.Flags &= ~InGuiPerFameFlag::RightMouseButtonPressed;
		s_Context->PerFrameData.Flags &= ~InGuiPerFameFlag::ReleaseHandled;
		s_Context->PerFrameData.Flags |=  InGuiPerFameFlag::RightMouseButtonReleased;
		if (s_Context->PerFrameData.ModifiedWindow)
		{
			s_Context->PerFrameData.ModifiedWindow->Flags &= ~(InGuiWindowFlag::Moved | InGuiWindowFlag::LeftResizing | InGuiWindowFlag::RightResizing | InGuiWindowFlag::TopResizing | InGuiWindowFlag::BottomResizing);
			s_Context->PerFrameData.ModifiedWindow = nullptr;
			auto& app = Application::Get();
			app.GetWindow().SetCursor(XYZ_ARROW_CURSOR);
		}
		for (auto window : s_Context->Windows)
		{
			if ((window.second->Flags & InGuiWindowFlag::Resized) && window.second->OnResizeCallback)
			{
				window.second->OnResizeCallback(window.second->Size);	
			}
			if (window.second->NodeWindow)
			{
				auto& spec = window.second->NodeWindow->FBO->GetSpecification();
				spec.Width = window.second->Size.x;
				spec.Height = window.second->Size.y;
				window.second->NodeWindow->FBO->Resize();
			}
			window.second->Flags &= ~InGuiWindowFlag::Resized;
		}
		return false;
	}

	bool InGui::OnMouseScroll()
	{
		InGuiWindow* window = s_Context->PerFrameData.EventReceivingWindow;
		if (window && (window->Flags & InGuiWindowFlag::Visible))
		{
			return true;
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


	bool InGui::IsKeyPressed(int key)
	{
		return s_Context->PerFrameData.KeyCode == key;
	}

	InGuiWindow* InGui::GetCurrentWindow()
	{
		return s_Context->PerFrameData.CurrentWindow;
	}

	InGuiNodeWindow* InGui::GetCurrentNodeWindow()
	{
		return s_Context->PerFrameData.CurrentNodeWindow;
	}

	glm::vec2& InGui::MouseRelativePosition(const InGuiWindow& window, const glm::vec3& cameraPos)
	{
		auto& mousePos = s_Context->PerFrameData.MousePosition;
		return glm::vec2{ mousePos.x + (window.Size.x / 2 * cameraPos.x), mousePos.y + (window.Size.y / 2 * cameraPos.y) };
	}
	InGuiNodeWindow* InGui::GetNodeWindow(uint32_t id)
	{
		auto it = s_Context->NodeWindows.find(id);
		if (it != s_Context->NodeWindows.end())
			return it->second;
		return nullptr;
	}

	InGuiWindow* InGui::GetWindow(uint32_t id)
	{		
		auto it = s_Context->Windows.find(id);
		if (it != s_Context->Windows.end())
		{
			it->second->Flags |= InGuiWindowFlag::Initialized;
			return it->second;
		}
		return nullptr;	
	}

	
	InGuiWindow* InGui::createWindow(uint32_t id, const glm::vec2& position, const glm::vec2& size)
	{
		InGuiWindow* window = new InGuiWindow;
		window->Position = position;
		window->Size = size;
		window->ID = id;
		window->Flags |= InGuiWindowFlag::Modified;
		window->Flags |= InGuiWindowFlag::EventListener;
		window->Flags |= InGuiWindowFlag::Visible;
		window->Flags |= InGuiWindowFlag::AutoPosition;
		window->Flags |= InGuiWindowFlag::Initialized;

		s_Context->Windows.insert({ id,window });
		return window;
	}
	
	InGuiNodeWindow* InGui::createNodeWindow(uint32_t id, const glm::vec2& position, const glm::vec2& size)
	{
		InGuiNodeWindow* window = new InGuiNodeWindow;
		
		window->RenderWindow = GetWindow(id);
		if (!window->RenderWindow)
			window->RenderWindow = createWindow(id, position, size);

		window->RenderWindow->NodeWindow = window;

		window->FBO = FrameBuffer::Create({ (uint32_t)window->RenderWindow->Size.x, (uint32_t)window->RenderWindow->Size.y });
		window->FBO->CreateColorAttachment(FrameBufferFormat::RGBA16F);
		window->FBO->CreateDepthAttachment();
		window->FBO->Resize();


		s_Context->NodeWindows.insert({ id,window });
	
		return window;
	}

	InGuiNode* InGui::createNode(uint32_t id, const glm::vec2& position, const glm::vec2& size)
	{
		auto nodeWindow = s_Context->PerFrameData.CurrentNodeWindow;
		InGuiNode* node = new InGuiNode;
		node->ID = id;
		node->Position = position;
		node->Size = size;
		nodeWindow->Nodes.insert({ id,node });
		return node;
	}

	InGuiNode* InGui::getNode(uint32_t id)
	{
		auto nodeWindow = s_Context->PerFrameData.CurrentNodeWindow;
		auto it = nodeWindow->Nodes.find(id);
		if (it == nodeWindow->Nodes.end())
			return nullptr;

		return it->second;
	}
	

	bool InGui::detectResize(InGuiWindow& window)
	{
		if (!(window.Flags & InGuiWindowFlag::Docked) && !(window.Flags & InGuiWindowFlag::Moved))
		{
			auto& app = Application::Get();
			auto& frameData = s_Context->PerFrameData;
			glm::vec2 offset = { 10,10 };
			auto& mousePos = frameData.MousePosition;
			bool resized = false;

			// Right side
			if (mousePos.x >= window.Position.x + window.Size.x - offset.x)
			{
				window.Flags |= InGuiWindowFlag::RightResizing;
				app.GetWindow().SetCursor(XYZ_HRESIZE_CURSOR);
				resized = true;
			}
			// Left side
			else if (mousePos.x <= window.Position.x + offset.x)
			{
				window.Flags |= InGuiWindowFlag::LeftResizing;
				app.GetWindow().SetCursor(XYZ_HRESIZE_CURSOR);
				resized = true;
			}
			// Bottom side
			if (mousePos.y <= window.Position.y + offset.y)
			{
				window.Flags |= InGuiWindowFlag::BottomResizing;
				app.GetWindow().SetCursor(XYZ_VRESIZE_CURSOR);
				resized = true;
			}
			// Top side
			else if (mousePos.y >= window.Position.y + window.Size.y - offset.y + InGuiWindow::PanelSize)
			{
				window.Flags |= InGuiWindowFlag::TopResizing;
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
			&& !(window.Flags & InGuiWindowFlag::Moved))
		{
			window.Flags |= InGuiWindowFlag::Moved;
			window.Flags |= InGuiWindowFlag::Modified;
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
			window.Flags ^= InGuiWindowFlag::Collapsed;
			window.Flags |= InGuiWindowFlag::Modified;
			return true;
		}
		return false;
	}
	void InGui::resolveResize(InGuiWindow& window)
	{
		auto& mousePos = s_Context->PerFrameData.MousePosition;
		auto& frameData = s_Context->PerFrameData;
		if (!(window.Flags & InGuiWindowFlag::Docked))
		{
			window.Flags |= InGuiWindowFlag::Modified;
			if (window.Size.x > window.MinimalWidth)
			{
				if (window.Flags & InGuiWindowFlag::RightResizing)
				{
					window.Size.x = mousePos.x - window.Position.x;
				}
				else if (window.Flags & InGuiWindowFlag::LeftResizing)
				{
					window.Size.x = window.Position.x + window.Size.x - mousePos.x;
					window.Position.x = mousePos.x;
				}
			}
			else
			{
				window.Size.x += InGuiWindow::PanelSize;
				window.Flags &= ~InGuiWindowFlag::LeftResizing;
				window.Flags &= ~InGuiWindowFlag::RightResizing;
			}

			if (window.Flags & InGuiWindowFlag::BottomResizing)
			{
				window.Size.y = window.Position.y + window.Size.y - mousePos.y;
				window.Position.y = mousePos.y;
			}
			else if (window.Flags & InGuiWindowFlag::TopResizing)
			{
				window.Size.y = mousePos.y - window.Position.y - InGuiWindow::PanelSize;
			}
		}
	}
	void InGui::resolveMove(InGuiWindow& window)
	{
		if (window.Flags & InGuiWindowFlag::Moved)
		{
			auto& frameData = s_Context->PerFrameData;
			glm::vec2 pos = frameData.MousePosition - frameData.ModifiedWindowMouseOffset;
			window.Position = { pos.x, pos.y - window.Size.y };
			s_Context->DockSpace->RemoveWindow(&window);
			s_Context->DockSpace->m_DockSpaceVisible = true;
			window.Flags |= InGuiWindowFlag::Modified;
		}
	}

	bool InGui::resolveLeftClick(bool handle)
	{
		if ((s_Context->PerFrameData.Flags & InGuiPerFameFlag::LeftMouseButtonPressed) &&
			!(s_Context->PerFrameData.Flags & InGuiPerFameFlag::ClickHandled))
		{
			if (handle)
				s_Context->PerFrameData.Flags |= InGuiPerFameFlag::ClickHandled;
			return true;
		}	
		return false;
	}

	bool InGui::resolveRightClick(bool handle)
	{
		if ((s_Context->PerFrameData.Flags & InGuiPerFameFlag::RightMouseButtonPressed) &&
			!(s_Context->PerFrameData.Flags & InGuiPerFameFlag::ClickHandled))
		{
			if (handle)
				s_Context->PerFrameData.Flags |= InGuiPerFameFlag::ClickHandled;
			return true;
		}
		return false;
	}

	bool InGui::resolveLeftRelease(bool handle)
	{
		if ((s_Context->PerFrameData.Flags & InGuiPerFameFlag::LeftMouseButtonReleased) &&
			!(s_Context->PerFrameData.Flags & InGuiPerFameFlag::ReleaseHandled))
		{
			if (handle)
				s_Context->PerFrameData.Flags |= InGuiPerFameFlag::ReleaseHandled;
			return true;
		}
		return false;
	}

	bool InGui::resolveRightRelease(bool handle)
	{
		if ((s_Context->PerFrameData.Flags & InGuiPerFameFlag::RightMouseButtonReleased) &&
			!(s_Context->PerFrameData.Flags & InGuiPerFameFlag::ReleaseHandled))
		{
			if (handle)
				s_Context->PerFrameData.Flags |= InGuiPerFameFlag::ReleaseHandled;
			return true;
		}
		return false;
	}

	std::string GetID(const std::string& src)
	{
		bool found = false;
		size_t split = 0;
		
		for (int64_t i = src.size() - 1; i >= 0; --i)
		{
			if (src[i] == ' ')
			{
				split = (size_t)i;
				found = true;
				break;
			}
		}
		XYZ_ASSERT(found, "Id was not found");
		return src.substr(split + 1, src.size() - split);
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
			std::unordered_map<uint32_t, std::vector<InGuiWindow*>> windowMap;

			// Load windows
			auto it = ini.begin();
			while (it->first != "dockspace" && it != ini.end())
			{
				uint32_t windowID = atoi(it->first.c_str());
				windows[windowID] = new InGuiWindow();
				windows[windowID]->Position = StringToVec2(it->second.get("position"));
				windows[windowID]->Size = StringToVec2(it->second.get("size"));
				int32_t id = atoi(it->second.get("docknode").c_str());
				
				if (id != -1)
					windowMap[id].push_back(windows[windowID]);

				if ((bool)atoi(it->second.get("collapsed").c_str()))
					windows[windowID]->Flags |= InGuiWindowFlag::Collapsed;

				windows[windowID]->Flags |= InGuiWindowFlag::Modified;
				windows[windowID]->Flags |= InGuiWindowFlag::EventListener;
				windows[windowID]->Flags |= InGuiWindowFlag::Visible;
				windows[windowID]->Flags |= InGuiWindowFlag::AutoPosition;
				
				it++;
			}


			std::unordered_map<uint32_t, InGuiDockNode*> dockMap;
			std::unordered_map<uint32_t, int32_t> parentMap;

			uint32_t maxID = 0;
			// Load dockspace
			auto el = it->second.begin();
			while (el != it->second.end())
			{
				std::string nodeID = GetID(el->first);
				uint32_t id = atoi(nodeID.c_str());
				if (id > maxID)
					maxID = id;

				glm::vec2 pos = StringToVec2(it->second.get("node position " + nodeID));
				glm::vec2 size = StringToVec2(it->second.get("node size " + nodeID));
				int32_t parentID = atoi(it->second.get("node parent " + nodeID).c_str());
				if (parentID != -1)
					parentMap[id] = parentID;

				dockMap[id] = new InGuiDockNode(pos, size, id);
				dockMap[id]->Split = (SplitAxis)atoi(it->second.get("node split " + nodeID).c_str());
				dockMap[id]->Dock = (DockPosition)atoi(it->second.get("node dockposition " + nodeID).c_str());

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
			for (auto winVector : windowMap)
			{
				for (auto win : winVector.second)
				{
					win->Flags |= InGuiWindowFlag::Docked;
					win->Flags &= ~InGuiWindowFlag::Visible;
					win->DockNode = dockMap[winVector.first];
					win->DockNode->VisibleWindow = win;
					win->DockNode->Windows.push_back(win);
				}	
				// Set last set window to visible
				dockMap[winVector.first]->VisibleWindow->Flags |= InGuiWindowFlag::Visible;
			}
			
			// Setup new dockspace and root
			s_Context->DockSpace = new InGuiDockSpace(dockMap[0]);
			s_Context->DockSpace->m_NextNodeID = maxID + 1;
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
		s_Context->DockSpace->updateAll(s_Context->DockSpace->m_Root);

		for (auto& it : s_Context->Windows)
		{
			if (it.second->Flags & InGuiWindowFlag::Initialized)
			{
				std::string pos = std::to_string(it.second->Position.x) + "," + std::to_string(it.second->Position.y);
				std::string size = std::to_string(it.second->Size.x) + "," + std::to_string(it.second->Size.y);
				std::string id = std::to_string(it.first);
				ini[id]["position"] = pos;
				ini[id]["size"] = size;
				bool collapsed = (it.second->Flags & InGuiWindowFlag::Collapsed);
				ini[id]["collapsed"] = std::to_string(collapsed);
				if (it.second->DockNode)
					ini[id]["Docknode"] = std::to_string(it.second->DockNode->ID);
				else
					ini[id]["Docknode"] = std::to_string(-1);
			}
		}
		SaveDockSpace(ini, s_Context->DockSpace->m_Root);
		file.write(ini);
	}
}