#include "stdafx.h"
#include "InGui.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Renderer/Renderer.h"
#include "InGuiFactory.h"

#include <ini.h>


#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>


#include <stb_image.h>


namespace XYZ {
	InGuiContext* InGui::s_Context = nullptr;


	static int32_t s_NoneSelection = -1;
	static constexpr uint32_t sc_SliderFloatSize = 5;
	static constexpr size_t sc_TextBufferSize = 260; // Windows max size of path

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

	static void MoveVertices(InGuiVertex* vertices, const glm::vec2& position, uint32_t offset, uint32_t count)
	{
		for (uint32_t i = offset; i < count + offset; ++i)
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


	static void HandleInputNumber(char* text, int & length, int code, int mode, bool capslock)
	{
		int currentSize = length;

		switch (code)
		{
		case ToUnderlying(KeyCode::XYZ_KEY_BACKSPACE):
			if (length)
				length--;
		}
		if (length < sc_TextBufferSize)
		{
			switch (code)
			{
			case ToUnderlying(KeyCode::XYZ_KEY_0):
				text[length++] = ('0');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_1):
				text[length++] = ('1');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_2):
				text[length++] = ('2');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_3):
				text[length++] = ('3');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_4):
				text[length++] = ('4');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_5):
				text[length++] = ('5');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_6):
				text[length++] = ('6');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_7):
				text[length++] = ('7');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_8):
				text[length++] = ('8');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_9):
				text[length++] = ('9');
				break;
			case ToUnderlying(KeyCode::XYZ_KEY_PERIOD):
				text[length++] = ('.');
				break;
			}
		}	

		if (currentSize != length)
			text[length] = '\0';
	}

	static bool IsOutside(const InGuiWindow& window, const glm::vec2& size, const glm::vec2& position)
	{
		return !(position.x > window.Position.x && position.x + size.x < window.Position.x + window.Size.x
			 && position.y > window.Position.y && position.y + size.y < window.Position.y + window.Size.y);
	}
	static bool IsPointOutside(const InGuiWindow& window, const glm::vec2& point)
	{
		return !(point.x > window.Position.x && point.x < window.Position.x + window.Size.x
			&& point.y > window.Position.y && point.y < window.Position.y + window.Size.y);
	}
	static bool HandleRowPosition(InGuiPerFrameData& frameData, const glm::vec2& uiSize, glm::vec2& position)
	{
		float offsetX = uiSize.x + frameData.WindowSpaceOffset.x + frameData.ItemOffset;
		float offsetY = frameData.WindowSpaceOffset.y - uiSize.y;

		float windowRightSide = frameData.CurrentWindow->Position.x + frameData.CurrentWindow->Size.x;
		float windowBottomSide = frameData.CurrentWindow->Position.y;

		if (offsetY <= windowBottomSide)
			return false;

		if (offsetX <= windowRightSide)
		{
			if (uiSize.y > frameData.MaxHeightInRow)
				frameData.MaxHeightInRow = uiSize.y;

			position.x = (frameData.WindowSpaceOffset.x + frameData.ItemOffset);
			position.y = (frameData.WindowSpaceOffset.y - uiSize.y - frameData.ItemOffset);

			frameData.WindowSpaceOffset.x += (uiSize.x + frameData.ItemOffset);
			return true;
		}
		else if (frameData.CurrentWindow->Flags & InGuiWindowFlag::ForceNewLine)
		{
			InGui::Separator();
			return HandleRowPosition(frameData, uiSize, position);
		}

		return false;
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
		s_Context->GetPerFrameData().WindowSize.x = Input::GetWindowSize().first;
		s_Context->GetPerFrameData().WindowSize.y = Input::GetWindowSize().second;
		s_Context->GetPerFrameData().Flags |= InGuiPerFrameFlag::ClickHandled;
		loadDockSpace();
	}
	void InGui::Destroy()
	{
		saveDockSpace();

		for (auto window : s_Context->Windows)
		{
			delete window.second;
		}
	
		delete s_Context->DockSpace;
		delete s_Context;
	}
	void InGui::BeginFrame()
	{
		auto [mx,my] = Input::GetMousePosition();
		s_Context->GetPerFrameData().EventReceivingWindow = nullptr;
		s_Context->GetPerFrameData().MousePosition = MouseToWorld({ mx,my }, s_Context->GetPerFrameData().WindowSize);
		s_Context->DockSpace->Begin();
		s_Context->ActivateMainFrame();
	}
	void InGui::EndFrame()
	{
		if (s_Context->GetPerFrameData().ModifiedWindow)
		{
			resolveResize(*s_Context->GetPerFrameData().ModifiedWindow);
			resolveMove(*s_Context->GetPerFrameData().ModifiedWindow);
		}

		InGuiRenderer::BeginScene({ s_Context->GetPerFrameData().ViewProjectionMatrix, s_Context->GetPerFrameData().WindowSize });
		InGuiRenderer::SetTexturePairs(s_Context->GetPerFrameData().TexturePairs);
		InGuiRenderer::SetMaterial(s_Context->RenderConfiguration.InMaterial);
		
		for (auto mesh : s_Context->GetRenderQueue().GetMeshes())
		{
			InGuiRenderer::SetMaterial(mesh->Material);
			InGuiRenderer::SubmitUI(*mesh);
		}
		for (auto mesh : s_Context->GetRenderQueue().GetLineMeshes())
		{
			InGuiRenderer::SubmitLineMesh(*mesh);
		}
		
		s_Context->DockSpace->Render(s_Context->GetPerFrameData().MousePosition, s_Context->GetPerFrameData(), s_Context->RenderConfiguration);
		s_Context->DockSpace->Update(s_Context->GetPerFrameData().MousePosition);

		InGuiRenderer::Flush();
		InGuiRenderer::FlushLines();
		InGuiRenderer::EndScene();
		Renderer::WaitAndRender();

		s_Context->RenderConfiguration.NumTexturesInUse = InGuiRenderConfiguration::DefaultTextureCount;
		s_Context->GetPerFrameData().TexturePairs.clear();
	
		s_Context->GetRenderQueue().Reset();
		
		
		// Clean codes
		s_Context->GetPerFrameData().KeyCode = ToUnderlying(KeyCode::XYZ_KEY_NONE);
		s_Context->GetPerFrameData().Mode = ToUnderlying(KeyMode::XYZ_MOD_NONE);
		s_Context->GetPerFrameData().Code = ToUnderlying(MouseCode::XYZ_MOUSE_NONE);
	}
	void InGui::BeginSubFrame()
	{
		s_Context->ActivateSubFrame();
	}
	void InGui::EndSubFrame()
	{
		InGuiRenderer::BeginScene({ s_Context->GetPerFrameData().ViewProjectionMatrix, s_Context->GetPerFrameData().WindowSize });
		InGuiRenderer::SetTexturePairs(s_Context->GetPerFrameData().TexturePairs);
		InGuiRenderer::SetMaterial(s_Context->RenderConfiguration.InMaterial);

		for (auto mesh : s_Context->GetRenderQueue().GetMeshes())
		{
			InGuiRenderer::SetMaterial(mesh->Material);
			InGuiRenderer::SubmitUI(*mesh);
		}
		for (auto mesh : s_Context->GetRenderQueue().GetLineMeshes())
		{
			InGuiRenderer::SubmitLineMesh(*mesh);
		}

		InGuiRenderer::FlushLines();
		InGuiRenderer::Flush();
		InGuiRenderer::EndScene();
		Renderer::WaitAndRender();

		s_Context->RenderConfiguration.NumTexturesInUse = InGuiRenderConfiguration::DefaultTextureCount;
		
		s_Context->GetPerFrameData().TexturePairs.clear();
		s_Context->GetRenderQueue().Reset();

		// Clean codes
		s_Context->GetPerFrameData().KeyCode = ToUnderlying(KeyCode::XYZ_KEY_NONE);
		s_Context->GetPerFrameData().Mode = ToUnderlying(KeyMode::XYZ_MOD_NONE);
		s_Context->GetPerFrameData().Code = ToUnderlying(MouseCode::XYZ_MOUSE_NONE);
		s_Context->ActivateMainFrame();
	}
	bool InGui::Begin(uint32_t id, const char* name, const glm::vec2& position, const glm::vec2& size)
	{
		XYZ_ASSERT(!s_Context->GetPerFrameData().CurrentWindow, "Missing end call");
		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = GetWindow(id);
		if (!window)
			window = createWindow(id, position, size);

		window->Name = name;

		frameData.CurrentWindow = window;
		frameData.ActiveMesh = &window->Mesh;
		frameData.ActiveLineMesh = &window->LineMesh;
		
		if (!(window->Flags & InGuiWindowFlag::Visible))
			return false;

		frameData.WindowSpaceOffset.x = window->Position.x;
		frameData.WindowSpaceOffset.y = window->Position.y + window->Size.y;
		frameData.ActiveMesh->Material = renderConfig.InMaterial;

		// Check if window is hoovered
		glm::vec2 winSize = window->Size + glm::vec2(0.0f, InGuiWindow::PanelSize);
		if (Collide(window->Position, winSize, frameData.MousePosition))
		{
			window->Flags |= InGuiWindowFlag::Modified;
			window->Flags |= InGuiWindowFlag::Hoovered;

			s_Context->GetRenderQueue().PushOverlay(&*frameData.ActiveMesh, frameData.ActiveLineMesh);
			
			if (window->Flags & InGuiWindowFlag::EventListener)
				frameData.EventReceivingWindow = window;
		}	
		else
			s_Context->GetRenderQueue().Push(frameData.ActiveMesh, frameData.ActiveLineMesh);
		
	

		// Check if was modified
		if (window->Flags & InGuiWindowFlag::Modified)
			InGuiFactory::GenerateWindow(name, *window, renderConfig);

		
		return !(window->Flags & InGuiWindowFlag::Collapsed);
	}
	void InGui::End()
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().CurrentWindow, "Missing begin call");
		
		s_Context->GetPerFrameData().CurrentWindow->Flags &= ~InGuiWindowFlag::Modified;
		s_Context->GetPerFrameData().CurrentWindow->Flags &= ~InGuiWindowFlag::Hoovered;
		s_Context->GetPerFrameData().CurrentWindow->Flags &= ~InGuiWindowFlag::LeftClicked;
		s_Context->GetPerFrameData().CurrentWindow->Flags &= ~InGuiWindowFlag::RightClicked;
		s_Context->GetPerFrameData().ResetWindowData();
	}

	uint8_t InGui::BeginPopup(const char* name, glm::vec2& position, const glm::vec2& size, bool& open)
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;
	
		uint8_t result = 0;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec4 color = renderConfig.Color[InGuiRenderConfiguration::DEFAULT_COLOR];
			if (window->Flags & InGuiWindowFlag::AutoPosition)
				if (!HandleRowPosition(frameData, size, position))
					return false;
			
		
			if (Collide(position, size, frameData.MousePosition))
			{
				color = renderConfig.Color[InGuiRenderConfiguration::HOOVER_COLOR];
				result |= InGuiReturnType::Hoovered;
				if (ResolveLeftClick())
				{
					open = !open;
					result |= InGuiReturnType::Clicked;
				}
			}

			InGuiFactory::GenerateButton(position, size, color, name, *frameData.ActiveMesh, renderConfig);
			
			frameData.PopupSize = size;
			frameData.PopupOffset.x = position.x;
			frameData.PopupOffset.y = position.y - size.y;
			
		}
		return result;
	}

	uint8_t InGui::PopupItem(const char* name)
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;
		
		uint8_t result = 0;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec4 color = renderConfig.Color[InGuiRenderConfiguration::DEFAULT_COLOR];
			glm::vec2 position = { frameData.PopupOffset.x, frameData.PopupOffset.y };
			if (Collide(position, frameData.PopupSize, frameData.MousePosition))
			{
				result |= InGuiReturnType::Hoovered;
				color = renderConfig.Color[InGuiRenderConfiguration::HOOVER_COLOR];
				if (ResolveLeftClick())
				{
					result |= InGuiReturnType::Clicked;
				}
			}
			InGuiFactory::GenerateButton(position, frameData.PopupSize, color, name, *frameData.ActiveMesh, renderConfig);
		}
		
		frameData.PopupItemCount++;
		frameData.PopupOffset.y -= frameData.PopupSize.y;
		
		return result;
	}

	uint8_t InGui::PopupExpandItem(const char* name, bool& open)
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;
		frameData.PopupItemCount = 0;
		
		uint8_t result = 0;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec4 color = renderConfig.Color[InGuiRenderConfiguration::DEFAULT_COLOR];
			glm::vec2 position = { frameData.PopupOffset.x, frameData.PopupOffset.y };
			if (Collide(position, frameData.PopupSize, frameData.MousePosition))
			{
				result |= InGuiReturnType::Hoovered;
				color = renderConfig.Color[InGuiRenderConfiguration::HOOVER_COLOR];
				if (ResolveLeftClick())
				{
					open = !open;
					result |= InGuiReturnType::Clicked;
				}
			}
			
			frameData.PopupOffset.x += frameData.PopupSize.x;
			
			InGuiFactory::GenerateButton(position, frameData.PopupSize, color, name, *frameData.ActiveMesh, renderConfig);
		}
		return result;
	}

	void InGui::PopupExpandEnd()
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().CurrentWindow, "Missing begin call");
		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
	
		frameData.PopupOffset.x -= frameData.PopupSize.x;
		frameData.PopupOffset.y += frameData.PopupSize.y * ((int)frameData.PopupItemCount - 1);
		frameData.PopupItemCount = 0;
	}

	void InGui::EndPopup()
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().CurrentWindow, "Missing begin call");
		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		frameData.PopupOffset = { 0, 0 };
	}


	uint8_t InGui::MenuBar(const char* name, float width, bool& open)
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;

		uint8_t result = 0;
		if (window->Flags & InGuiWindowFlag::Modified)
		{		
			if (window->Flags & InGuiWindowFlag::MenuEnabled)
			{
				glm::vec4 color = renderConfig.Color[InGuiRenderConfiguration::DEFAULT_COLOR];
				glm::vec2 size = { width,InGuiWindow::PanelSize };
				glm::vec2 position = { window->Position.x + frameData.MenuBarOffset.x, window->Position.y + window->Size.y };
				if (Collide(position, size, frameData.MousePosition))
				{
					result |= InGuiReturnType::Hoovered;
					color = renderConfig.Color[InGuiRenderConfiguration::HOOVER_COLOR];
					if (ResolveLeftClick())
					{
						open = !open;
						result |= InGuiReturnType::Clicked;
					}
				}
				

				InGuiFactory::GenerateMenuBar(position, size, color, name, frameData, renderConfig);

				frameData.MenuBarOffset.x += width;
				frameData.MenuBarOffset.y = position.y - InGuiWindow::PanelSize;
				frameData.MenuItemOffset = position.x;
			}
		}
		return result;
	}

	uint8_t InGui::MenuItem(const char* name, const glm::vec2& size)
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;
		
		uint8_t result = 0;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			if (window->Flags & InGuiWindowFlag::MenuEnabled)
			{
				glm::vec4 color = renderConfig.Color[InGuiRenderConfiguration::DEFAULT_COLOR];
				glm::vec2 position = { frameData.MenuItemOffset,frameData.MenuBarOffset.y };
				if (Collide(position, size, frameData.MousePosition))
				{
					result |= InGuiReturnType::Hoovered;
					color = renderConfig.Color[InGuiRenderConfiguration::HOOVER_COLOR];
					if (ResolveLeftClick())
					{
						result |= InGuiReturnType::Clicked;
					}
				}
				InGuiFactory::GenerateMenuBar(position, size, color, name, frameData, renderConfig);
				frameData.MenuBarOffset.y -= size.y;
			}
		}
		return result;
	}

	uint8_t InGui::BeginGroup(const char* name, const glm::vec2& position, bool& open)
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;
		
		uint8_t result = 0;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec2 winPos = window->Position;
			glm::vec2 winSize = window->Size;
			glm::vec2 panelPos = position;
				
			frameData.WindowSpaceOffset.y -= InGuiWindow::PanelSize;
			panelPos = { winPos.x, frameData.WindowSpaceOffset.y };

			glm::vec2 minButtonPos = { panelPos.x + 5, panelPos.y };
			glm::vec4 color = renderConfig.Color[InGuiRenderConfiguration::DEFAULT_COLOR];
			if (Collide(minButtonPos, { InGuiWindow::PanelSize,InGuiWindow::PanelSize }, frameData.MousePosition))
			{
				result |= InGuiReturnType::Hoovered;
				color = renderConfig.Color[InGuiRenderConfiguration::HOOVER_COLOR];
				if (ResolveLeftClick())
				{
					result |= InGuiReturnType::Clicked;
					open = !open;
				}
			}
			InGuiFactory::GenerateGroup(panelPos, color, name, open, frameData, renderConfig);
		}

		return result;
	}

	void InGui::EndGroup()
	{
		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		frameData.WindowSpaceOffset.y -= InGuiWindow::PanelSize;
	}


	static bool ResolveSpaceWithText(const char* text, const glm::vec2 size, const glm::vec4& color, glm::vec2& position, const InGuiRenderConfiguration& renderConfig, InGuiPerFrameData& frameData)
	{
		XYZ_ASSERT(strlen(text) < sc_TextBufferSize, "Maximum length of text is ", sc_TextBufferSize);
		InGuiWindow* window = frameData.CurrentWindow;
		InGuiMesh& mesh = *frameData.ActiveMesh;

		auto info = InGuiFactory::GenerateText(color, text, 1000.0f, frameData.TempVertices, renderConfig);
	
		if (window->Flags & InGuiWindowFlag::AutoPosition)
		{
			if (!HandleRowPosition(frameData, { size.x + info.Size.x + 5, size.y }, position))
				return false;
		}
		glm::vec2 textOffset = { size.x + 5,(size.y / 2) - ((float)info.Size.y / 1.5f) };
		MoveVertices(frameData.TempVertices, position + textOffset, 0, info.Count * 4);
		for (uint32_t i = 0; i < info.Count * 4; ++i)
			mesh.Vertices.push_back(frameData.TempVertices[i]);

		return true;
	}

	uint8_t InGui::Button(const char* name, const glm::vec2& size)
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;

		uint8_t result = 0;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec4 color = renderConfig.Color[InGuiRenderConfiguration::DEFAULT_COLOR];
			glm::vec2 position; //= HandleWindowSpacing(size, frameData);
			if (HandleRowPosition(frameData, size, position))
			{
				if (Collide(position, size, frameData.MousePosition))
				{
					result |= InGuiReturnType::Hoovered;
					color = renderConfig.Color[InGuiRenderConfiguration::HOOVER_COLOR];
					if (ResolveLeftClick())
					{
						result |= InGuiReturnType::Clicked;
					}
				}
				InGuiFactory::GenerateButton(position, size, color, name, *frameData.ActiveMesh, renderConfig);
			}
		}
		return result;
	}

	uint8_t InGui::Checkbox(const char* name, const glm::vec2 position, const glm::vec2& size, bool& value)
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;

		uint8_t result = 0;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec4 color = renderConfig.Color[InGuiRenderConfiguration::DEFAULT_COLOR];
			glm::vec2 pos = position;
			if (ResolveSpaceWithText(name, size, color, pos, renderConfig, frameData))
			{
				if (Collide(pos, size, frameData.MousePosition))
				{
					result |= InGuiReturnType::Hoovered;
					color = renderConfig.Color[InGuiRenderConfiguration::HOOVER_COLOR];
					if (ResolveLeftClick())
					{
						value = !value;
						result |= InGuiReturnType::Clicked;
					}
				}
				InGuiFactory::GenerateCheckbox(pos, size, color, name, value, *frameData.ActiveMesh, renderConfig);
			}
		}
		return result;
	}

	uint8_t InGui::Slider(const char* name, const glm::vec2 position, const glm::vec2& size, float& value, float valueScale)
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;
		
		uint8_t result = 0;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec4 color = renderConfig.Color[InGuiRenderConfiguration::DEFAULT_COLOR];	
			glm::vec2 handleSize = { size.x , size.y * 2 };
			glm::vec2 pos = position;
			if (ResolveSpaceWithText(name, handleSize, color, pos, renderConfig, frameData))
			{
				if (Collide(pos, handleSize, frameData.MousePosition))
				{
					result |= InGuiReturnType::Hoovered;
					color = renderConfig.Color[InGuiRenderConfiguration::HOOVER_COLOR];
					if (ResolveLeftClick(false))
					{
						float start = pos.x;
						value = (frameData.MousePosition.x - start) / valueScale;
						result |= InGuiReturnType::Clicked;
					}
				}
			}
			InGuiFactory::GenerateSlider(pos, size, color, name,value * valueScale,frameData.WindowSpaceOffset, *frameData.ActiveMesh, renderConfig);
		}

		return result;
	}

	uint8_t InGui::Image(const char* name, uint32_t rendererID,const glm::vec2& position, const glm::vec2& size,  const glm::vec4& texCoords, float tilingFactor)
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;
	
		uint8_t result = 0;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec2 pos = position;
			if (window->Flags & InGuiWindowFlag::AutoPosition)
				if (!HandleRowPosition(frameData, size, pos))
					return false;
					
			if (Collide(pos, size, frameData.MousePosition))
			{
				result |= InGuiReturnType::Hoovered;
				if (ResolveLeftClick(false))
					result |= InGuiReturnType::Clicked;
			}
			InGuiFactory::GenerateImage(pos, size, renderConfig.Color[InGuiRenderConfiguration::DEFAULT_COLOR], texCoords, rendererID, *frameData.ActiveMesh, frameData.TexturePairs, renderConfig, tilingFactor);		
		}
		else
		{
			SubmitTexture(rendererID, frameData.TexturePairs, renderConfig);
		}

		return result;
	}

	uint8_t InGui::TextArea(const char* name, std::string& text, const glm::vec2& position, const glm::vec2& size, bool& modified)
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;

		uint8_t result = 0;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec4 color = renderConfig.Color[InGuiRenderConfiguration::DEFAULT_COLOR];
			glm::vec2 pos = position;
			if (ResolveSpaceWithText(name, size, color, pos, renderConfig, frameData))
			{
				if (Collide(pos, size, frameData.MousePosition))
				{
					result |= InGuiReturnType::Hoovered;
					if (ResolveLeftClick())
					{
						result |= InGuiReturnType::Clicked;
						modified = !modified;
					}
				}
				else if (ResolveLeftClick(false) || ResolveRightClick(false))
				{
					result |= InGuiReturnType::Clicked;
					modified = false;
				}

				if (modified)
				{
					color = renderConfig.Color[InGuiRenderConfiguration::HOOVER_COLOR];
					HandleInputText(text, frameData.KeyCode, frameData.Mode, frameData.CapslockEnabled);
				}


				InGuiFactory::GenerateTextArea(pos, size, color, name, text.c_str(), frameData.WindowSpaceOffset, *frameData.ActiveMesh, renderConfig);
			}
		}
		
		return result;
	}

	uint8_t InGui::Float(uint32_t count, const char* name, float* values, int32_t* lengths, const glm::vec2& position, const glm::vec2& size, int32_t& selected)
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;

		uint8_t result = 0;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			float offset = 5.0f;
			glm::vec4 color = renderConfig.Color[InGuiRenderConfiguration::DEFAULT_COLOR];
			glm::vec2 pos = position;
			if (ResolveSpaceWithText(name, { (size.x + offset) * count, size.y }, color, pos, renderConfig, frameData))
			{
				if (ResolveLeftClick(false))
				{
					selected = s_NoneSelection;
					glm::vec2 tmpPos = pos;
					for (int i = 0; i < count; ++i)
					{
						if (Collide(tmpPos, size, frameData.MousePosition))
						{
							result |= (InGuiReturnType::Clicked & InGuiReturnType::Hoovered);
							frameData.Flags |= InGuiPerFrameFlag::ClickHandled;
							selected = i;
							break;
						}
						tmpPos.x += size.x + offset;
					}
				}
				for (int i = 0; i < count; ++i)
				{
					glm::vec4 color = renderConfig.Color[InGuiRenderConfiguration::DEFAULT_COLOR];
					char buffer[sc_TextBufferSize];
					int ret = snprintf(buffer, sizeof(buffer), "%f", values[i]);
					buffer[lengths[i]] = '\0';
					if (i == selected)
					{
						color = renderConfig.Color[InGuiRenderConfiguration::HOOVER_COLOR];
						HandleInputNumber(buffer, lengths[i], frameData.KeyCode, frameData.Mode, frameData.CapslockEnabled);
						values[i] = atof(buffer);
					}
					InGuiFactory::GenerateTextArea(pos, size, color, name, buffer, frameData.WindowSpaceOffset, *frameData.ActiveMesh, renderConfig);
					pos.x += size.x + offset;
				}
			}
		}
		return result;
	}

	uint8_t InGui::Icon(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, uint32_t textureID)
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().CurrentWindow, "Missing begin call");
		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;
		
		uint8_t result = 0;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec4 color = renderConfig.Color[InGuiRenderConfiguration::DEFAULT_COLOR];
			glm::vec2 pos = position;
			if (window->Flags & InGuiWindowFlag::AutoPosition)
				if (!HandleRowPosition(frameData, size, pos))
					return result;
	
			if (Collide(pos, size, frameData.MousePosition))
			{
				result |= InGuiReturnType::Hoovered;
				color = renderConfig.Color[InGuiRenderConfiguration::HOOVER_COLOR];
				if (ResolveLeftClick(true))
				{
					result |= InGuiReturnType::Clicked;
				}
			}
			InGuiFactory::GenerateIcon(*frameData.ActiveMesh, pos, size, color, subTexture, textureID);		
		}
		return result;
	}

	uint8_t InGui::Icon(const char* name, const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, uint32_t textureID)
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;

		uint8_t result = 0;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec4 color = renderConfig.Color[InGuiRenderConfiguration::DEFAULT_COLOR];
			glm::vec2 pos = { 0,0 };
			if (ResolveSpaceWithText(name, size, color, pos, renderConfig, frameData))
			{
				if (Collide(pos, size, frameData.MousePosition))
				{
					result |= InGuiReturnType::Hoovered;
					color = renderConfig.Color[InGuiRenderConfiguration::HOOVER_COLOR];
					if (ResolveLeftClick(true))
					{
						result |= InGuiReturnType::Clicked;
					}
				}
				InGuiFactory::GenerateIcon(*frameData.ActiveMesh, pos, size, color, subTexture, textureID);
			}
		}
		return result;
	}


	uint8_t InGui::Text(const char* text, const glm::vec2& scale, const glm::vec4& color)
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().CurrentWindow, "Missing begin call");
		XYZ_ASSERT(strlen(text) < sc_TextBufferSize, "Maximum length of text is ", sc_TextBufferSize);

		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;

		uint8_t result = 0;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			auto info = InGuiFactory::GenerateText(color, text, 1000.0f, frameData.TempVertices, renderConfig);

			glm::vec2 pos = { 0,0 };
			if (window->Flags & InGuiWindowFlag::AutoPosition)
				if (!HandleRowPosition(frameData, info.Size, pos))
					return false;

			
			MoveVertices(frameData.TempVertices, pos, 0, info.Count * 4);
			for (uint32_t i = 0; i < info.Count * 4; ++i)
				frameData.ActiveMesh->Vertices.push_back(frameData.TempVertices[i]);

			if (Collide(pos, info.Size, frameData.MousePosition))
			{
				result |= InGuiReturnType::Hoovered;
				if (ResolveLeftClick())
					result |= InGuiReturnType::Clicked;
			}		
		}

		return result;
	}

	uint8_t InGui::ColorPicker4(const char* name, const glm::vec2& size, glm::vec4& pallete, glm::vec4& color)
	{
		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;

		uint8_t result = 0;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec2 pos = { 0,0 };
			if (window->Flags & InGuiWindowFlag::AutoPosition)
				if (!HandleRowPosition(frameData, size, pos))
					return result;

			Separator();
			if (ColorPallete4("", { size.x, 25.0f }, pallete))
			{
				result |= InGuiReturnType::Clicked;
			}
			Separator();
			InGuiFactory::GenerateColorPicker4(pos, size, pallete, *frameData.ActiveMesh, renderConfig);
			
			char buffer[sc_SliderFloatSize + 6];
			sprintf(buffer, "R: %.*f", sc_SliderFloatSize, color.x);
			if (Slider(buffer, {}, { size.x,15 }, color.x, size.x))
				result |= InGuiReturnType::Clicked;
			Separator();

			sprintf(buffer, "G: %.*f", sc_SliderFloatSize, color.y);
			if (Slider(buffer, {}, { size.x,15 }, color.y, size.x))
				result |= InGuiReturnType::Clicked;
			Separator();

			sprintf(buffer, "B: %.*f", sc_SliderFloatSize, color.z);
			if (Slider(buffer, {}, { size.x,15 }, color.z, size.x))
				result |= InGuiReturnType::Clicked;
			Separator();

			if (Collide(pos, size, frameData.MousePosition))
			{
				result |= InGuiReturnType::Hoovered;
				if (ResolveLeftClick())
				{
					result |= InGuiReturnType::Clicked;
					color = CalculatePixelColor(pallete, pos, size, frameData);
				}
			}
		}
		return result;
	}

	uint8_t InGui::ColorPallete4(const char* name, const glm::vec2& size, glm::vec4& color)
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().CurrentWindow, "Missing begin call");

		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;

		uint8_t result = 0;
		if (window->Flags & InGuiWindowFlag::Modified)
		{
			glm::vec2 pos = { 0,0 };
			if (window->Flags & InGuiWindowFlag::AutoPosition)
				if (!HandleRowPosition(frameData, size, pos))
					return result;
			
			InGuiFactory::Generate6SegmentColorRectangle(pos, size, *frameData.ActiveMesh, renderConfig);
			if (Collide(pos, size, frameData.MousePosition))
			{
				result |= InGuiReturnType::Hoovered;
				if (ResolveLeftClick())
				{
					result |= InGuiReturnType::Clicked;
					color = ColorFrom6SegmentColorRectangle(pos, size, frameData.MousePosition);
				}
			}	
		}
		return result;
	}

	

	bool InGui::RenderWindow(uint32_t id,const char* name, uint32_t rendererID, const glm::vec2& position, const glm::vec2& size)
	{
		XYZ_ASSERT(!s_Context->GetPerFrameData().CurrentWindow, "Missing end call");

		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = GetWindow(id);
		if (!window)
			window = createWindow(id, position, size);
		
		window->Name = name;
		frameData.CurrentWindow = window;
		frameData.ActiveMesh = &window->Mesh;
		frameData.ActiveLineMesh = &window->LineMesh;
		if (!(window->Flags & InGuiWindowFlag::Visible))
			return false;

		frameData.WindowSpaceOffset.x = window->Position.x;
		frameData.WindowSpaceOffset.y = window->Position.y + window->Size.y;
		frameData.ActiveMesh->Material = renderConfig.InMaterial;

		// Check if window is hoovered
		glm::vec2 winSize = window->Size + glm::vec2(0.0f, InGuiWindow::PanelSize);
		if (Collide(window->Position, winSize, frameData.MousePosition))
		{
			window->Flags |= InGuiWindowFlag::Modified;
			window->Flags |= InGuiWindowFlag::Hoovered;
			s_Context->GetRenderQueue().PushOverlay(&*frameData.ActiveMesh, frameData.ActiveLineMesh);

			if (window->Flags & InGuiWindowFlag::EventListener)
				frameData.EventReceivingWindow = window;
		}
		else
			s_Context->GetRenderQueue().Push(&*frameData.ActiveMesh, frameData.ActiveLineMesh);
		
	
		if (window->Flags & InGuiWindowFlag::Modified)
			InGuiFactory::GenerateRenderWindow(name, *window, rendererID, frameData, renderConfig);
		else
			SubmitTexture(rendererID, frameData.TexturePairs, renderConfig);
		
		return (window->Flags & InGuiWindowFlag::Hoovered);
	}


	void InGui::Separator()
	{
		auto& frameData = s_Context->GetPerFrameData();
		frameData.WindowSpaceOffset.x = frameData.CurrentWindow->Position.x;
		frameData.WindowSpaceOffset.y -= frameData.MaxHeightInRow + frameData.ItemOffset;
		frameData.MaxHeightInRow = 0.0f;
	}

	uint8_t InGui::BeginNode(const char* name, const glm::vec2& position, const glm::vec2& size)
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().ActiveMesh, "No active mesh");
		XYZ_ASSERT(!s_Context->GetPerFrameData().CurrentWindow, "Node can not be related to window, set mesh");
		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
			
		uint8_t result = 0;
		glm::vec4 color = renderConfig.Color[InGuiRenderConfiguration::DEFAULT_COLOR];
		if (Collide(position, size, frameData.MousePosition))
		{
			result |= InGuiReturnType::Hoovered;
			color = renderConfig.Color[InGuiRenderConfiguration::HOOVER_COLOR];
			if (ResolveLeftClick())
			{
				result |= InGuiReturnType::Clicked;
			}
		}
		InGuiFactory::GenerateButton(position, size, color, name, *frameData.ActiveMesh, renderConfig);	
		
		return result;
	}

	void InGui::PushArrow(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& size)
	{
		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();	
		InGuiFactory::GenerateArrowLine(*frameData.ActiveMesh, *frameData.ActiveLineMesh, p0, p1, size, s_Context->RenderConfiguration);
	}


	glm::vec4 InGui::Selector(bool& selecting)
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().CurrentWindow, "Missing begin call");
		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;

		InGuiWindow* window = frameData.CurrentWindow;

		if (window->Flags & InGuiWindowFlag::Hoovered)
		{
			if (ResolveLeftClick())
			{
				frameData.SelectedPoint = frameData.MousePosition;
				selecting = true;
			}
			if (selecting)
			{
				if (ResolveLeftRelease(false) || (frameData.MousePosition.y >= window->Position.y + window->Size.y))
					selecting = false;

				glm::vec2 size = frameData.MousePosition - frameData.SelectedPoint;	
				InGuiFactory::GenerateFrame(*frameData.ActiveLineMesh, frameData.SelectedPoint, size, renderConfig.Color[InGuiRenderConfiguration::LINE_COLOR]);
				return glm::vec4(frameData.SelectedPoint, frameData.SelectedPoint + size);
			}
		}
		return glm::vec4(0);
	}

	void InGui::Selection(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		XYZ_ASSERT(s_Context->GetPerFrameData().CurrentWindow, "Missing begin call");
		InGuiPerFrameData& frameData = s_Context->GetPerFrameData();
		InGuiRenderConfiguration& renderConfig = s_Context->RenderConfiguration;
		InGuiWindow* window = frameData.CurrentWindow;

		if (window->Flags & InGuiWindowFlag::Modified)
		{
			InGuiFactory::GenerateFrame(*frameData.ActiveLineMesh, position, size, color);
		}
	}
	
	bool InGui::OnWindowResize(const glm::vec2& winSize)
	{
		s_Context->GetPerFrameData().WindowSize = winSize;
		s_Context->RenderConfiguration.InMaterial->Set("u_ViewportSize", s_Context->GetPerFrameData().WindowSize);
		s_Context->DockSpace->OnWindowResize(winSize);
		return false;
	}

	bool InGui::OnLeftMouseButtonPress()
	{
		if (s_Context->DockSpace->OnLeftMouseButtonPress())
			return true;

		s_Context->GetPerFrameData().Code = ToUnderlying(MouseCode::XYZ_MOUSE_BUTTON_LEFT);
		s_Context->GetPerFrameData().Flags |= InGuiPerFrameFlag::LeftMouseButtonPressed;
		s_Context->GetPerFrameData().Flags &= ~InGuiPerFrameFlag::ClickHandled;
		s_Context->GetPerFrameData().Flags &= ~InGuiPerFrameFlag::LeftMouseButtonReleased;

		InGuiWindow* window = s_Context->GetPerFrameData().EventReceivingWindow;
		if (window && (window->Flags & InGuiWindowFlag::Visible))
		{		
			if (detectCollapse(*window))
			{
				return (window->Flags & InGuiWindowFlag::EventBlocking);
			}
			if (!(window->Flags & InGuiWindowFlag::Collapsed))
			{
				window->Flags |= InGuiWindowFlag::LeftClicked;
			}

			return (window->Flags & InGuiWindowFlag::EventBlocking);
		}	
		return false;
	}

	bool InGui::OnRightMouseButtonPress()
	{
		if (s_Context->DockSpace->OnRightMouseButtonPress(s_Context->GetPerFrameData().MousePosition))
			return true;
		s_Context->GetPerFrameData().Code = ToUnderlying(MouseCode::XYZ_MOUSE_BUTTON_RIGHT);
		s_Context->GetPerFrameData().Flags |=  InGuiPerFrameFlag::RightMouseButtonPressed;
		s_Context->GetPerFrameData().Flags &= ~InGuiPerFrameFlag::ClickHandled;
		s_Context->GetPerFrameData().Flags &= ~InGuiPerFrameFlag::RightMouseButtonReleased;
		
		InGuiWindow* window = s_Context->GetPerFrameData().EventReceivingWindow;
		if (window && (window->Flags & InGuiWindowFlag::Visible))
		{		
			if (detectResize(*window))
			{
				s_Context->GetPerFrameData().Flags |= InGuiPerFrameFlag::ClickHandled;
				s_Context->GetPerFrameData().ModifiedWindow = window;
			}
			else if(detectMoved(*window))
			{
				s_Context->GetPerFrameData().Flags |= InGuiPerFrameFlag::ClickHandled;
				s_Context->GetPerFrameData().ModifiedWindow = window;
			}		
			else
			{
				window->Flags |= InGuiWindowFlag::RightClicked;
			}
			return (window->Flags & InGuiWindowFlag::EventBlocking);
		}		
		
		return false;
	}

	bool InGui::OnLeftMouseButtonRelease()
	{
		s_Context->GetPerFrameData().Flags |=  InGuiPerFrameFlag::LeftMouseButtonReleased;
		s_Context->GetPerFrameData().Flags &= ~InGuiPerFrameFlag::ReleaseHandled;
		s_Context->GetPerFrameData().Flags &= ~InGuiPerFrameFlag::LeftMouseButtonPressed;
		s_Context->GetPerFrameData().Code = ToUnderlying(MouseCode::XYZ_MOUSE_NONE);
		if (s_Context->GetPerFrameData().ModifiedWindow)
		{
			s_Context->GetPerFrameData().ModifiedWindow->Flags &= ~(InGuiWindowFlag::Moved | InGuiWindowFlag::LeftResizing | InGuiWindowFlag::RightResizing | InGuiWindowFlag::TopResizing | InGuiWindowFlag::BottomResizing);
			s_Context->GetPerFrameData().ModifiedWindow = nullptr;
		}
	
		return false;
	}

	bool InGui::OnRightMouseButtonRelease()
	{
		s_Context->GetPerFrameData().Code = ToUnderlying(MouseCode::XYZ_MOUSE_NONE);
		s_Context->DockSpace->OnRightMouseButtonRelease(s_Context->GetPerFrameData().ModifiedWindow, s_Context->GetPerFrameData().MousePosition);
		s_Context->GetPerFrameData().Flags &= ~InGuiPerFrameFlag::RightMouseButtonPressed;
		s_Context->GetPerFrameData().Flags &= ~InGuiPerFrameFlag::ReleaseHandled;
		s_Context->GetPerFrameData().Flags |=  InGuiPerFrameFlag::RightMouseButtonReleased;
		if (s_Context->GetPerFrameData().ModifiedWindow)
		{
			s_Context->GetPerFrameData().ModifiedWindow->Flags &= ~(InGuiWindowFlag::Moved | InGuiWindowFlag::LeftResizing | InGuiWindowFlag::RightResizing | InGuiWindowFlag::TopResizing | InGuiWindowFlag::BottomResizing);
			s_Context->GetPerFrameData().ModifiedWindow = nullptr;
			auto& app = Application::Get();
			app.GetWindow().SetStandardCursor(XYZ_ARROW_CURSOR);
		}

		return false;
	}

	bool InGui::OnKeyPress(int key, int mod)
	{
		if (mod == ToUnderlying(KeyMode::XYZ_MOD_CAPS_LOCK))
			s_Context->GetPerFrameData().CapslockEnabled = !s_Context->GetPerFrameData().CapslockEnabled;

		s_Context->GetPerFrameData().KeyCode = key;
		s_Context->GetPerFrameData().Mode = mod;
		return false;
	}

	bool InGui::OnKeyRelease()
	{
		s_Context->GetPerFrameData().KeyCode = ToUnderlying(KeyCode::XYZ_KEY_NONE);
		s_Context->GetPerFrameData().Mode = ToUnderlying(KeyMode::XYZ_MOD_NONE);
		return false;
	}


	bool InGui::IsKeyPressed(int key)
	{
		return s_Context->GetPerFrameData().KeyCode == key;
	}

	void InGui::SetInGuiMesh(InGuiMesh* mesh, InGuiLineMesh* lineMesh, bool overlay)
	{
		XYZ_ASSERT(mesh && lineMesh, "Meshes can not be null");
		mesh->Material = s_Context->RenderConfiguration.InMaterial;
		s_Context->GetPerFrameData().ActiveMesh = mesh;
		s_Context->GetPerFrameData().ActiveLineMesh = lineMesh;
		if (overlay)
			s_Context->GetRenderQueue().PushOverlay(mesh, lineMesh);
		else
			s_Context->GetRenderQueue().Push(mesh, lineMesh);
	}

	void InGui::SetViewProjection(const glm::mat4& viewProjection)
	{
		s_Context->GetPerFrameData().ViewProjectionMatrix = viewProjection;
	}

	void InGui::SetMousePosition(const glm::vec2& mousePos)
	{
		s_Context->GetPerFrameData().MousePosition = mousePos;
	}

	void InGui::SetUIOffset(float offset)
	{
		s_Context->GetPerFrameData().ItemOffset = offset;
	}

	InGuiWindow* InGui::GetCurrentWindow()
	{
		return s_Context->GetPerFrameData().CurrentWindow;
	}


	InGuiRenderConfiguration& InGui::GetRenderConfiguration()
	{
		return s_Context->RenderConfiguration;
	}

	const glm::vec2& InGui::GetMousePosition()
	{
		return s_Context->GetPerFrameData().MousePosition;
	}

	glm::vec2 InGui::GetWorldPosition(const InGuiWindow& window, const glm::vec3& cameraPos, float aspectRatio, float zoomLevel)
	{
		auto [x, y] = Input::GetMousePosition();
		auto [width, height] = Input::GetWindowSize();
		x -= ((float)width / 2.0f) - fabs(window.Position.x);
		y -= ((float)height / 2.0f) - window.Position.y - window.Size.y;

		float cameraBoundWidth = (aspectRatio * zoomLevel) * 2;
		float cameraBoundHeight = zoomLevel * 2;
		auto pos = cameraPos;

		x = (x / window.Size.x) * cameraBoundWidth - cameraBoundWidth * 0.5f;
		y = cameraBoundHeight * 0.5f - (y / window.Size.y) * cameraBoundHeight;

		return { x + pos.x , y + pos.y };
	}

	glm::vec2 InGui::GetInGuiPosition(const InGuiWindow& window, const glm::vec3& cameraPos, float aspectRatio, float zoomLevel)
	{
		auto [x, y] = Input::GetMousePosition();
		auto [width, height] = Input::GetWindowSize();
		x -= ((float)width / 2.0f) - fabs(window.Position.x);
		y -= ((float)height / 2.0f) - window.Position.y - window.Size.y;

		float cameraBoundWidth = (aspectRatio * zoomLevel) * 2;
		float cameraBoundHeight = zoomLevel * 2;
		auto pos = cameraPos;

		x = (x / window.Size.x) * cameraBoundWidth - cameraBoundWidth * 0.5f;
		y = cameraBoundHeight * 0.5f - (y / window.Size.y) * cameraBoundHeight;
		x *= ((float)width / 2.0f);
		y *= ((float)height / 2.0f);
		x += pos.x * ((float)width / 2.0f);
		y += pos.y * ((float)height / 2.0f);

		return { x + pos.x , y + pos.y };
	}

	glm::vec2 InGui::MouseRelativePosition(const InGuiWindow& window, const glm::vec3& cameraPos)
	{
		auto& mousePos = s_Context->GetPerFrameData().MousePosition;
		return glm::vec2{ mousePos.x + (window.Size.x / 2 * cameraPos.x), mousePos.y + (window.Size.y / 2 * cameraPos.y) };
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
		window->Flags |= InGuiWindowFlag::Dockable;
		window->Flags |= InGuiWindowFlag::EventBlocking;

		s_Context->Windows.insert({ id,window });
		return window;
	}
	
	bool InGui::detectResize(InGuiWindow& window)
	{
		if (!(window.Flags & InGuiWindowFlag::Docked) && !(window.Flags & InGuiWindowFlag::Moved))
		{
			auto& app = Application::Get();
			auto& frameData = s_Context->GetPerFrameData();
			glm::vec2 offset = { 10,10 };
			auto& mousePos = frameData.MousePosition;
			bool resized = false;

			// Right side
			if (mousePos.x >= window.Position.x + window.Size.x - offset.x)
			{
				window.Flags |= InGuiWindowFlag::RightResizing;
				app.GetWindow().SetStandardCursor(XYZ_HRESIZE_CURSOR);
				resized = true;
			}
			// Left side
			else if (mousePos.x <= window.Position.x + offset.x)
			{
				window.Flags |= InGuiWindowFlag::LeftResizing;
				app.GetWindow().SetStandardCursor(XYZ_HRESIZE_CURSOR);
				resized = true;
			}
			// Bottom side
			if (mousePos.y <= window.Position.y + offset.y)
			{
				window.Flags |= InGuiWindowFlag::BottomResizing;
				app.GetWindow().SetStandardCursor(XYZ_VRESIZE_CURSOR);
				resized = true;
			}
			// Top side
			else if (mousePos.y >= window.Position.y + window.Size.y - offset.y + InGuiWindow::PanelSize)
			{
				window.Flags |= InGuiWindowFlag::TopResizing;
				app.GetWindow().SetStandardCursor(XYZ_VRESIZE_CURSOR);
				resized = true;
			}
			
			return resized;
		}
		return false;
	}
	bool InGui::detectMoved(InGuiWindow& window)
	{
		if (s_Context->GetPerFrameData().MousePosition.y >= window.Position.y + window.Size.y
			&& !(window.Flags & InGuiWindowFlag::Moved))
		{
			window.Flags |= InGuiWindowFlag::Moved;
			window.Flags |= InGuiWindowFlag::Modified;
			s_Context->GetPerFrameData().ModifiedWindowMouseOffset = s_Context->GetPerFrameData().MousePosition - window.Position - glm::vec2{ 0, window.Size.y };
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

		if (Collide(minButtonPos, minButtonSize, s_Context->GetPerFrameData().MousePosition))
		{
			window.Flags ^= InGuiWindowFlag::Collapsed;
			window.Flags |= InGuiWindowFlag::Modified;
			return true;
		}
		return false;
	}
	void InGui::resolveResize(InGuiWindow& window)
	{
		auto& mousePos = s_Context->GetPerFrameData().MousePosition;
		auto& frameData = s_Context->GetPerFrameData();
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
			auto& frameData = s_Context->GetPerFrameData();
			glm::vec2 pos = frameData.MousePosition - frameData.ModifiedWindowMouseOffset;
			window.Position = { pos.x, pos.y - window.Size.y };
			s_Context->DockSpace->RemoveWindow(&window);
			if (window.Flags & InGuiWindowFlag::Dockable)
				s_Context->DockSpace->m_DockSpaceVisible = true;
			window.Flags |= InGuiWindowFlag::Modified;
		}
	}

	bool InGui::ResolveLeftClick(bool handle)
	{
		if ((s_Context->GetPerFrameData().Flags & InGuiPerFrameFlag::LeftMouseButtonPressed) &&
			!(s_Context->GetPerFrameData().Flags & InGuiPerFrameFlag::ClickHandled))
		{
			if (handle)
				s_Context->GetPerFrameData().Flags |= InGuiPerFrameFlag::ClickHandled;
			return true;
		}	
		return false;
	}

	bool InGui::ResolveRightClick(bool handle)
	{
		if ((s_Context->GetPerFrameData().Flags & InGuiPerFrameFlag::RightMouseButtonPressed) &&
			!(s_Context->GetPerFrameData().Flags & InGuiPerFrameFlag::ClickHandled))
		{
			if (handle)
				s_Context->GetPerFrameData().Flags |= InGuiPerFrameFlag::ClickHandled;
			return true;
		}
		return false;
	}

	bool InGui::ResolveLeftRelease(bool handle)
	{
		if ((s_Context->GetPerFrameData().Flags & InGuiPerFrameFlag::LeftMouseButtonReleased) &&
			!(s_Context->GetPerFrameData().Flags & InGuiPerFrameFlag::ReleaseHandled))
		{
			if (handle)
				s_Context->GetPerFrameData().Flags |= InGuiPerFrameFlag::ReleaseHandled;
			return true;
		}
		return false;
	}

	bool InGui::ResolveRightRelease(bool handle)
	{
		if ((s_Context->GetPerFrameData().Flags & InGuiPerFrameFlag::RightMouseButtonReleased) &&
			!(s_Context->GetPerFrameData().Flags & InGuiPerFrameFlag::ReleaseHandled))
		{
			if (handle)
				s_Context->GetPerFrameData().Flags |= InGuiPerFrameFlag::ReleaseHandled;
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

	static void LoadWindowMaps(
		mINI::INIStructure::const_iterator& it,
		std::unordered_map<uint32_t, InGuiWindow*>& windowMap,
		std::unordered_map<uint32_t, std::vector<InGuiWindow*>>& windowDockMap,
		const mINI::INIStructure& ini)
	{
		while (it->first != "dockspace" && it != ini.end())
		{
			uint32_t windowID = atoi(it->first.c_str());
			windowMap[windowID] = new InGuiWindow();
			windowMap[windowID]->Position = StringToVec2(it->second.get("position"));
			windowMap[windowID]->Size = StringToVec2(it->second.get("size"));
			int32_t id = atoi(it->second.get("docknode").c_str());

			if (id != -1)
				windowDockMap[id].push_back(windowMap[windowID]);

			if ((bool)atoi(it->second.get("collapsed").c_str()))
				windowMap[windowID]->Flags |= InGuiWindowFlag::Collapsed;

			windowMap[windowID]->Flags |= InGuiWindowFlag::Modified;
			windowMap[windowID]->Flags |= InGuiWindowFlag::EventListener;
			windowMap[windowID]->Flags |= InGuiWindowFlag::Visible;
			windowMap[windowID]->Flags |= InGuiWindowFlag::AutoPosition;
			windowMap[windowID]->Flags |= InGuiWindowFlag::EventBlocking;

			it++;
		}
	}

	static uint32_t LoadAndSetupDockSpaceMaps(
		mINI::INIStructure::const_iterator& it,
		std::unordered_map<uint32_t, InGuiDockNode*>& dockMap,
		std::unordered_map<uint32_t, int32_t>& parentMap
	)
	{
		static constexpr uint32_t numPropertiesDockNode = 5;
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
		return maxID;
	}


	static void SetupWindowMap(
		std::unordered_map<uint32_t, std::vector<InGuiWindow*>>& windowMap,
		std::unordered_map<uint32_t, InGuiDockNode*>& dockMap
		)
	{
		// Setup windows
		for (auto winVector : windowMap)
		{
			for (auto win : winVector.second)
			{
				win->Flags |= InGuiWindowFlag::Dockable;
				win->Flags |= InGuiWindowFlag::Docked;
				win->Flags &= ~InGuiWindowFlag::Visible;
				win->DockNode = dockMap[winVector.first];
				win->DockNode->VisibleWindow = win;
				win->DockNode->Windows.push_back(win);
			}
			// Set last set window to visible
			dockMap[winVector.first]->VisibleWindow->Flags |= InGuiWindowFlag::Visible;
		}
	}

	void InGui::loadDockSpace()
	{	
		mINI::INIFile file("ingui.ini");
		mINI::INIStructure ini;

		auto& frameData = s_Context->GetPerFrameData();
		

		frameData.WindowSize.x = (float)Input::GetWindowSize().first;
		frameData.WindowSize.y = (float)Input::GetWindowSize().second;

		if (file.read(ini))
		{
			std::unordered_map<uint32_t, std::vector<InGuiWindow*>> windowMap;
			std::unordered_map<uint32_t, InGuiDockNode*> dockMap;
			std::unordered_map<uint32_t, int32_t> parentMap;

			auto it = ini.begin();
			LoadWindowMaps(it, s_Context->Windows, windowMap, ini);
			uint32_t maxID = LoadAndSetupDockSpaceMaps(it, dockMap, parentMap);		
			SetupWindowMap(windowMap, dockMap);
				
			// Setup new dockspace and root
			s_Context->DockSpace = new InGuiDockSpace(dockMap[0]);
			s_Context->DockSpace->m_NextNodeID = maxID + 1;

			glm::vec2 scale = s_Context->GetPerFrameData().WindowSize / s_Context->DockSpace->m_Root->Size;
			s_Context->DockSpace->rescale(scale, s_Context->DockSpace->m_Root);
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
	
	void InGuiContext::ActivateSubFrame()
	{
		XYZ_ASSERT(!MainFrameData.PerFrameData.CurrentWindow, "Current window must be ended");
		// Copy main frame data
		SubFrameData.PerFrameData.WindowSize = MainFrameData.PerFrameData.WindowSize;
		SubFrameData.PerFrameData.MousePosition = MainFrameData.PerFrameData.MousePosition;
		SubFrameData.PerFrameData.Flags = MainFrameData.PerFrameData.Flags;
		CurrentFrameData = &SubFrameData;
	}

}