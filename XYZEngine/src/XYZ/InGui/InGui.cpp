#include "stdafx.h"
#include "InGui.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer.h"

#include "InGuiFactory.h"
#include "InGuiDockspace.h"

#include <glm/gtx/transform.hpp>

#include <ini.h>

namespace XYZ {

	InGuiContext* InGui::s_Context = nullptr;

	static char s_TextBuffer[_MAX_PATH];
	static char s_ModifyTextBuffer[_MAX_PATH];
	static uint32_t s_ModifyTextBufferIndex = 0;

	static float s_HighestInRow = 0.0f;
	static glm::vec2 s_LayoutOffset = glm::vec2(0.0f);

	static glm::vec2 StringToVec2(const std::string& src)
	{
		glm::vec2 val;
		size_t split = src.find(",", 0);

		val.x = std::stof(src.substr(0, split));
		val.y = std::stof(src.substr(split + 1, src.size() - split));

		return val;
	}

	template <typename Type>
	static bool TurnOffFlag(Type& num, Type flag)
	{
		bool isSet = (num & flag);
		num &= ~flag;
		return isSet;
	}

	template <typename Type>
	static bool TurnOnFlag(Type& num, Type flag)
	{
		bool isSet = (num & flag);
		num |= flag;
		return isSet;
	}

	static uint32_t FindNumCharacters(const char* source, float maxWidth, Ref<Font> font)
	{
		if (!source)
			return 0;

		float xCursor = 0.0f;
		uint32_t counter = 0;
		while (source[counter] != '\0')
		{
			auto& character = font->GetCharacter(source[counter]);
			if (xCursor + (float)character.XAdvance >= maxWidth)
				break;
		
			xCursor += character.XAdvance;
			counter++;
		}
		return counter;
	}

	static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
	{
		return (pos.x + size.x > point.x &&
			    pos.x		   < point.x&&
			    pos.y + size.y >  point.y &&
			    pos.y < point.y);
	}
	

	void InGui::Init()
	{
		s_Context = new InGuiContext();
		loadLayout();
	}

	void InGui::Destroy()
	{
		saveLayout();
		if (s_Context)
			delete s_Context;
	}

	void InGui::BeginFrame(const glm::mat4& viewProjectionMatrix)
	{
		XYZ_ASSERT(s_Context, "InGuiContext is not initialized");
		s_Context->FrameData.ViewProjectionMatrix = viewProjectionMatrix;	
		s_Context->FrameData.InputIndex = 0;


		glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
		viewMatrix = glm::inverse(viewMatrix);

		Renderer2D::BeginScene(s_Context->FrameData.ViewProjectionMatrix * viewMatrix);
		Renderer2D::SetMaterial(s_Context->RenderData.Material);
		InGuiDockspace::beginFrame(s_Context, (s_Context->FrameData.MovedWindowID != InGuiFrameData::NullID));
	}

	void InGui::EndFrame()
	{
		XYZ_ASSERT(s_Context, "InGuiContext is not initialized");
		InGuiDockspace::endFrame(s_Context);
		Renderer2D::SetMaterial(s_Context->RenderData.Material);
		for (auto winIt = s_Context->Windows.rbegin(); winIt != s_Context->Windows.rend(); ++winIt)
		{
			if (!IS_SET(winIt->Flags, InGuiWindowFlags::Docked))
			{
				for (auto it = winIt->Mesh.Quads.begin(); it != winIt->Mesh.Quads.end(); ++it)
				{
					Renderer2D::SubmitQuadNotCentered(it->Position, it->Size, it->TexCoord, it->TextureID, it->Color);
				}
				for (auto& line : winIt->Mesh.Lines)
				{
					Renderer2D::SubmitLine(line.P0, line.P1, line.Color);
				}
			}
		}

		
		Renderer2D::Flush();
		Renderer2D::FlushLines();
		Renderer2D::EndScene();
		Renderer::WaitAndRender();

		handleWindowMove();
		handleWindowResize();
	}


	void InGui::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		if (dispatcher.Dispatch<MouseButtonPressEvent>(&InGui::onMouseButtonPress))
		{ }
		else if (dispatcher.Dispatch<MouseButtonReleaseEvent>(&InGui::onMouseButtonRelease))
		{ }
		else if (dispatcher.Dispatch<MouseMovedEvent>(&InGui::onMouseMove))
		{ }
		else if (dispatcher.Dispatch<KeyTypedEvent>(&InGui::onKeyTyped))
		{ }
		else if (dispatcher.Dispatch<KeyPressedEvent>(&InGui::onKeyPressed))
		{ }
	}

	void InGui::SetLayout(uint32_t id, const InGuiLayout& layout)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[id];
		window.Layout = layout;
	}

	void InGui::Separator()
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];

		s_LayoutOffset.x = window.Position.x + window.Layout.LeftPadding;
		s_LayoutOffset.y += s_HighestInRow + window.Layout.TopPadding;
		s_HighestInRow = 0.0f;
	}

	bool InGui::eraseOutOfBorders(size_t oldQuadCount, const glm::vec2& genSize, InGuiWindow& window)
	{
		if (s_LayoutOffset.x + genSize.x >= window.Position.x + window.Size.x 
		 || s_LayoutOffset.y + genSize.y >= window.Position.y + window.Size.y)
		{
			s_LayoutOffset.x += genSize.x + window.Layout.SpacingX;
			window.Mesh.Quads.erase(window.Mesh.Quads.begin() + oldQuadCount, window.Mesh.Quads.end());
			return true;
		}
		else if (s_HighestInRow < genSize.y)
			s_HighestInRow = genSize.y;
		
		return false;
	}

	InGuiWindow& InGui::getInitializedWindow(uint32_t id, const glm::vec2& position, const glm::vec2& size)
	{
		if (s_Context->Windows.size() <= id)
		{
			s_Context->Windows.resize((size_t)id + 1);
		}
		if (!(s_Context->Windows[id].Flags & InGuiWindowFlags::Initialized))
		{
			s_Context->Windows[id].Position = position;
			s_Context->Windows[id].Size = size;
			s_Context->Windows[id].ID = id;
			s_Context->Windows[id].Flags |= (InGuiWindowFlags::Initialized | InGuiWindowFlags::EventBlocking);
		}
		return s_Context->Windows[id];
	}

	void InGui::handleWindowMove()
	{
		uint32_t id = s_Context->FrameData.MovedWindowID;
		if (id != InGuiFrameData::NullID)
		{
			s_Context->Windows[id].Position = s_Context->FrameData.MousePosition - s_Context->FrameData.MouseOffset;
		}
	}

	void InGui::handleWindowResize()
	{
		uint32_t id = s_Context->FrameData.ResizedWindowID;
		if (id != InGuiFrameData::NullID)
		{
			const glm::vec2& mousePos = s_Context->FrameData.MousePosition;
			InGuiWindow& window = s_Context->Windows[id];
			uint8_t resizeFlags = s_Context->FrameData.ResizeFlags;
			if (IS_SET(resizeFlags, InGuiResizeFlags::Left))
			{
				window.Size.x = window.Position.x + window.Size.x - mousePos.x;
				window.Position.x = mousePos.x;
			}
			else if (IS_SET(resizeFlags, InGuiResizeFlags::Right))
			{
				window.Size.x = mousePos.x - window.Position.x;
			}

			if (IS_SET(resizeFlags, InGuiResizeFlags::Bottom))
			{
				window.Size.y = mousePos.y - window.Position.y;
			}
		}
	}

	bool InGui::Begin(uint32_t id, const char* name, const glm::vec2& position, const glm::vec2& size)
	{	
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID == InGuiFrameData::NullID, "Missing end call");
		s_Context->FrameData.ActiveWindowID = id;
		InGuiWindow& window = getInitializedWindow(id, position, size);
		
		s_HighestInRow = 0.0f;
		s_LayoutOffset = glm::vec2(
			window.Position.x + window.Layout.LeftPadding, 
			window.Position.y + InGuiWindow::PanelHeight + window.Layout.TopPadding
		);

		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		if (IS_SET(window.Flags, InGuiWindowFlags::Hoovered))
		{
			color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
		}
		InGuiFactory::GenerateWindow(
			name, window, color, s_Context->RenderData, 
			s_Context->RenderData.SubTexture[InGuiRenderData::WINDOW], InGuiRenderData::TextureID
		);
		
		return !IS_SET(window.Flags, InGuiWindowFlags::Collapsed);
	}

	bool InGui::ImageWindow(uint32_t id, const char* name, const glm::vec2& position, const glm::vec2& size, Ref<SubTexture> subTexture)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID == InGuiFrameData::NullID, "Missing end call");
		s_Context->FrameData.ActiveWindowID = id;
		InGuiWindow& window = getInitializedWindow(id, position, size);

		s_HighestInRow = 0.0f;
		s_LayoutOffset = glm::vec2(
			window.Position.x + window.Layout.LeftPadding,
			window.Position.y + InGuiWindow::PanelHeight + window.Layout.TopPadding
		);

		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		if (IS_SET(window.Flags, InGuiWindowFlags::Hoovered))
		{
			color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
		}
		InGuiFactory::GenerateWindow(
			name, window, color, s_Context->RenderData,
			subTexture, Renderer2D::SetTexture(subTexture->GetTexture())
		);

		return !IS_SET(window.Flags, InGuiWindowFlags::Collapsed);
	}

	void InGui::End()
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		s_Context->FrameData.ActiveWindowID = InGuiFrameData::NullID;	
	}

	bool InGui::BeginGroup(const char* name, bool& open)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];

		size_t oldQuadCount = window.Mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		glm::vec2 size = { window.Size.x , InGuiWindow::PanelHeight };
		glm::vec2 buttonSize = glm::vec2(InGuiWindow::PanelHeight, InGuiWindow::PanelHeight);
		glm::vec2 pos = { window.Position.x , s_LayoutOffset.y };
		uint32_t subTextureIndex = InGuiRenderData::RIGHT_ARROW;
		if (open) subTextureIndex = InGuiRenderData::DOWN_ARROW;

		InGuiFactory::GenerateQuad(window.Mesh, color, size, pos, s_Context->RenderData, InGuiRenderData::BUTTON);
		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(name, window, color, buttonSize, pos, s_Context->RenderData, subTextureIndex);

		if (eraseOutOfBorders(oldQuadCount, buttonSize, window)) { return false; }

		if (Collide(pos, size, s_Context->FrameData.MousePosition))
		{
			window.Mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (Collide(pos, buttonSize, s_Context->FrameData.MousePosition))
			{
				if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
				{
					open = !open;
				}
			}
		}
		if (open) Separator();			
		return open;
	}

	uint8_t InGui::PushNode(const char* name, const glm::vec2& size, bool& open)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
	
		uint32_t returnType = 0;
		size_t oldQuadCount = window.Mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		glm::vec2 pos = s_LayoutOffset;
		uint32_t subTextureIndex = InGuiRenderData::RIGHT_ARROW;
		if (open) subTextureIndex = InGuiRenderData::DOWN_ARROW;

		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(name, window, color, size, pos, s_Context->RenderData, subTextureIndex);
		if (eraseOutOfBorders(oldQuadCount, genSize, window)) { return false; }

		if (Collide(pos, genSize, s_Context->FrameData.MousePosition))
		{
			if (Collide(pos, size, s_Context->FrameData.MousePosition))
			{
				window.Mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
				if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
				{
					open = !open;
				}
			}
			else // It collides with text
			{
				for (size_t i = oldQuadCount + 1; i < window.Mesh.Quads.size(); ++i)
					window.Mesh.Quads[i].Color = s_Context->RenderData.Color[InGuiRenderData::SELECT_COLOR];
				returnType |= InGuiReturnType::Hoovered;
				if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
				{
					returnType |= InGuiReturnType::Clicked;
				}
			}
		}
		s_LayoutOffset.y += genSize.y;
		return open;
	}

	uint8_t InGui::Dropdown(const char* name, const glm::vec2& size, bool& open)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];

		uint32_t returnType = 0;
		size_t oldQuadCount = window.Mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		glm::vec2 pos = s_LayoutOffset;

		glm::vec2 genSize = InGuiFactory::GenerateQuadWithTextLeft(name, window, color, size, pos, s_Context->RenderData, InGuiRenderData::BUTTON);
		if (eraseOutOfBorders(oldQuadCount, genSize, window)) { return false; }

		if (Collide(pos, genSize, s_Context->FrameData.MousePosition))
		{
			if (Collide(pos, size, s_Context->FrameData.MousePosition))
			{
				window.Mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
				if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
				{
					open = !open;
				}
			}
			else // It collides with text
			{
				for (size_t i = oldQuadCount + 1; i < window.Mesh.Quads.size(); ++i)
					window.Mesh.Quads[i].Color = s_Context->RenderData.Color[InGuiRenderData::SELECT_COLOR];
				returnType |= InGuiReturnType::Hoovered;
				if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
				{
					returnType |= InGuiReturnType::Clicked;
				}
			}
		}
		s_LayoutOffset.y += genSize.y;
		return open;
	}

	void InGui::BeginChildren()
	{		
		s_LayoutOffset.x += InGuiWindow::PanelHeight;
	}
	void InGui::EndChildren()
	{
		s_LayoutOffset.x -= InGuiWindow::PanelHeight;
	}


	uint8_t InGui::Button(const char* name, const glm::vec2& size)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];

		uint8_t returnType = 0;
		size_t oldQuadCount = window.Mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(name, window, color, size, s_LayoutOffset, s_Context->RenderData, InGuiRenderData::BUTTON);
				
		if (eraseOutOfBorders(oldQuadCount, genSize, window))
			return returnType;

		if (Collide(s_LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			window.Mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
				returnType |= InGuiReturnType::Clicked;
		}

		s_LayoutOffset.x += genSize.x + window.Layout.SpacingX;
		return returnType;
	}

	uint8_t InGui::Checkbox(const char* name, const glm::vec2& size, bool& val)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];

		uint8_t returnType = 0;
		size_t oldQuadCount = window.Mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		uint32_t subTextureIndex = InGuiRenderData::CHECKBOX_UNCHECKED;
		if (val) subTextureIndex = InGuiRenderData::CHECKBOX_CHECKED;
			
		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(name, window, color, size, s_LayoutOffset, s_Context->RenderData, subTextureIndex);

		if (eraseOutOfBorders(oldQuadCount, genSize, window))
			return returnType;

		if (Collide(s_LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			window.Mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
			{
				returnType |= InGuiReturnType::Clicked;
				val = !val;
			}
		}
		s_LayoutOffset.x += genSize.x + window.Layout.SpacingX;
		return returnType;
	}

	uint8_t InGui::Slider(const char* name, const glm::vec2& size, float& val)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		
		// Clamping
		if (val > 0.94f) val = 1.0f;
		if (val < 0.06f) val = 0.0f;

		int ret = snprintf(s_TextBuffer, sizeof(s_TextBuffer), "%f", val);
		if (ret < 0)
			val = 0.0f;

		uint8_t returnType = 0;
		size_t oldQuadCount = window.Mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(name, window, color, size, s_LayoutOffset, s_Context->RenderData, InGuiRenderData::SLIDER);
		glm::vec2 handleSize = glm::vec2(size.y, size.y);
		glm::vec2 handlePosition = s_LayoutOffset + glm::vec2((size.x - size.y) * val, 0.0f);
		InGuiFactory::GenerateQuadWithText(nullptr, window, color, handleSize, handlePosition, s_Context->RenderData, InGuiRenderData::SLIDER_HANDLE);
		InGuiFactory::GenerateTextCentered(s_TextBuffer, window, s_LayoutOffset, size, s_Context->RenderData, 1000);

		if (eraseOutOfBorders(oldQuadCount, genSize, window))
			return returnType;

		if (Collide(s_LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			window.Mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (IS_SET(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
			{
				val = (s_Context->FrameData.MousePosition.x - s_LayoutOffset.x) / size.x;
				returnType |= InGuiReturnType::Clicked;
			}
		}

		s_LayoutOffset.x += genSize.x + window.Layout.SpacingX;
		return returnType;
	}
	uint8_t InGui::Text(const char* text)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];

		uint8_t returnType = 0;
		size_t oldQuadCount = window.Mesh.Quads.size();
		glm::vec2 size = glm::vec2(500.0f);
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		

		glm::vec2 genPos = s_LayoutOffset + glm::vec2(0.0f, s_Context->RenderData.Font->GetLineHeight());
		glm::vec2 genSize = InGuiFactory::GenerateText(text, window, color, genPos, size, s_Context->RenderData);

		if (eraseOutOfBorders(oldQuadCount, genSize, window))
			return returnType;

		if (Collide(s_LayoutOffset, genSize, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			for (size_t i = oldQuadCount; i < window.Mesh.Quads.size(); ++i)
				window.Mesh.Quads[i].Color = s_Context->RenderData.Color[InGuiRenderData::SELECT_COLOR];
			
			if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
				returnType |= InGuiReturnType::Clicked;
		}

		s_LayoutOffset.x += genSize.x + window.Layout.SpacingX;
		return returnType;
	}

	uint8_t InGui::Float(const char* name, const glm::vec2& size, float& val)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		InGuiFrameData& data = s_Context->FrameData;

		uint8_t returnType = 0;
		size_t oldQuadCount = window.Mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		uint32_t maxCharacters = _MAX_PATH;
		char* buffer = s_TextBuffer; // By default text buffer

		if (data.InputIndex == data.HandleInput.size())
			data.HandleInput.push_back(false);

		// If input is about to be handled than use buffer for modifying values
		bool handleInput = data.HandleInput[data.InputIndex];
		if (handleInput)
		{
			color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			buffer = s_ModifyTextBuffer;

			s_ModifyTextBuffer[s_ModifyTextBufferIndex] = '.';
			val = (float)atof(s_ModifyTextBuffer);
			maxCharacters = s_ModifyTextBufferIndex;
		}
		else
		{
			// Otherwise just parse value as string to text buffer
			int ret = snprintf(buffer, sizeof(buffer), "%f", val);
			if (ret < 0) val = 0.0f;				
		}


		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(name, window, color, size, s_LayoutOffset, s_Context->RenderData, InGuiRenderData::BUTTON);
		InGuiFactory::GenerateTextCentered(buffer, window, s_LayoutOffset, size, s_Context->RenderData, maxCharacters);

		if (eraseOutOfBorders(oldQuadCount, genSize, window))
			return returnType;

		if (Collide(s_LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			window.Mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
			{
				returnType |= InGuiReturnType::Clicked;
				data.HandleInput[data.InputIndex].flip();
				if (data.HandleInput[data.InputIndex])
				{
					// This is called once when bit is fliped to true
					int ret = snprintf(s_TextBuffer, sizeof(s_TextBuffer), "%f", val);
					if (ret < 0) val = 0.0f;

					s_ModifyTextBufferIndex = FindNumCharacters(s_TextBuffer, size.x, s_Context->RenderData.Font);
					memcpy(s_ModifyTextBuffer, s_TextBuffer, s_ModifyTextBufferIndex);
				}
				for (uint32_t i = 0; i < s_Context->FrameData.HandleInput.size(); ++i)
				{
					if (i != data.InputIndex)
						data.HandleInput[i] = false;
				}
			}
		}
	
		data.InputIndex++;
		s_LayoutOffset.x += genSize.x + window.Layout.SpacingX;
		return returnType;
	}

	uint8_t InGui::Image(const glm::vec2& size, Ref<SubTexture> subTexture)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];

		uint8_t returnType = 0;
		size_t oldQuadCount = window.Mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		InGuiFactory::GenerateQuad(window.Mesh, color, size, s_LayoutOffset, subTexture, Renderer2D::SetTexture(subTexture->GetTexture()));

		if (eraseOutOfBorders(oldQuadCount, size, window))
			return returnType;

		if (Collide(s_LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			window.Mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
				returnType |= InGuiReturnType::Clicked;
		}

		s_LayoutOffset.x += size.x + window.Layout.SpacingX;
		return returnType;
	}

	void InGui::saveLayout()
	{
		mINI::INIFile file("ingui.ini");
		mINI::INIStructure ini;
		file.generate(ini);
		for (auto& win : s_Context->Windows)
		{
			if (IS_SET(win.Flags, InGuiWindowFlags::Initialized))
			{
				std::string pos = std::to_string(win.Position.x) + "," + std::to_string(win.Position.y);
				std::string size = std::to_string(win.Size.x) + "," + std::to_string(win.Size.y);
				std::string id = std::to_string(win.ID);
				std::string flags = std::to_string(win.Flags);
				ini[id]["position"] = pos;
				ini[id]["size"] = size;
				ini[id]["flags"] = flags;
			}
		}
		file.write(ini);
	}

	void InGui::loadLayout()
	{
		mINI::INIFile file("ingui.ini");
		mINI::INIStructure ini;
		
		if (file.read(ini))
		{
			for (auto it = ini.begin(); it != ini.end(); ++it)
			{
				size_t ID = atoi(it->first.c_str());
				if (ID >= s_Context->Windows.size())
					s_Context->Windows.resize(ID + 1);
				s_Context->Windows[ID].ID = ID;
				s_Context->Windows[ID].Position = StringToVec2(it->second.get("position"));
				s_Context->Windows[ID].Size = StringToVec2(it->second.get("size"));
				s_Context->Windows[ID].Flags = atoi(it->second.get("flags").c_str());
			}
		}
	}



	bool InGui::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_LEFT))
		{
			const glm::vec2& mousePos = s_Context->FrameData.MousePosition;
			if (InGuiDockspace::onMouseLeftPress(mousePos))
				return true;
			s_Context->FrameData.Flags |= InGuiInputFlags::LeftClicked;
			for (auto& window : s_Context->Windows)
			{
				if (IS_SET(window.Flags, InGuiWindowFlags::Initialized)
					&& Collide(window.Position, window.Size, mousePos))
				{
					bool handled = false;
					if (window.Position.y + InGuiWindow::PanelHeight >= mousePos.y) // Panel was hit
					{
						glm::vec2 minButtonPos = { window.Position.x + window.Size.x - InGuiWindow::PanelHeight, window.Position.y };
						if (Collide(minButtonPos, { InGuiWindow::PanelHeight, InGuiWindow::PanelHeight }, mousePos))
						{
							window.Flags ^= InGuiWindowFlags::Collapsed;
							handled = true;
						}
						else
						{
							s_Context->FrameData.MovedWindowID = window.ID;
							s_Context->FrameData.MouseOffset = mousePos - window.Position;
							handled = true;
							if (InGuiDockspace::removeWindow(window.ID))
								TurnOffFlag<uint16_t>(window.Flags, InGuiWindowFlags::Docked);
						}
					}
					else // Handle resize
					{
						if (mousePos.x < window.Position.x + 5.0f) // Left resize
						{
							s_Context->FrameData.ResizeFlags |= InGuiResizeFlags::Left;
							s_Context->FrameData.ResizedWindowID = window.ID;
							handled = true;
						}
						else if (mousePos.x > window.Position.x + window.Size.x - 5.0f) // Right resize
						{
							s_Context->FrameData.ResizeFlags |= InGuiResizeFlags::Right;
							s_Context->FrameData.ResizedWindowID = window.ID;
							handled = true;
						}
						if (mousePos.y > window.Position.y + window.Size.y - 5.0f) // Bottom
						{
							s_Context->FrameData.ResizeFlags |= InGuiResizeFlags::Bottom;
							s_Context->FrameData.ResizedWindowID = window.ID;
							handled = true;
							
						}
					}
					if (handled)
					{
						TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked);
						return IS_SET(window.Flags, InGuiWindowFlags::EventBlocking);
					}
				}
			}		
		}
		return false;
	}

	bool InGui::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		if (event.IsButtonReleased(MouseCode::MOUSE_BUTTON_LEFT))
		{
			bool handled = false;
			InGuiDockspace::onMouseLeftRelease();
			if (s_Context->FrameData.MovedWindowID != InGuiFrameData::NullID)
			{
				InGuiWindow& window = s_Context->Windows[s_Context->FrameData.MovedWindowID];
				if (InGuiDockspace::insertWindow(window.ID, s_Context->FrameData.MousePosition))
				{
					TurnOnFlag<uint16_t>(window.Flags, InGuiWindowFlags::Docked);
				}
			}

			s_Context->FrameData.MovedWindowID = InGuiFrameData::NullID;
			s_Context->FrameData.ResizedWindowID = InGuiFrameData::NullID;
			s_Context->FrameData.ResizeFlags = 0;
			s_Context->FrameData.Flags &= ~InGuiInputFlags::LeftClicked;

			
			for (auto& window : s_Context->Windows)
			{
				if (IS_SET(window.Flags, InGuiWindowFlags::Initialized)
					&& Collide(window.Position, window.Size, s_Context->FrameData.MousePosition))
				{
					if (window.Flags & InGuiWindowFlags::EventBlocking)
						handled = true;
				}
			}
			return handled;
		}

		return false;
	}

	bool InGui::onMouseMove(MouseMovedEvent& event)
	{
		s_Context->FrameData.MousePosition = { (float)event.GetX(), (float)event.GetY() };
		bool hoovered = false;
		for (auto& window : s_Context->Windows)
		{
			window.Flags &= ~InGuiWindowFlags::Hoovered;
			if (!hoovered
				&& IS_SET(window.Flags, InGuiWindowFlags::Initialized)
				&& !IS_SET(window.Flags, InGuiWindowFlags::Collapsed)
				&& Collide(window.Position, window.Size, s_Context->FrameData.MousePosition))
			{
				window.Flags |= InGuiWindowFlags::Hoovered;
				hoovered = true;
			}
		}
		return false;
	}
	bool InGui::onKeyTyped(KeyTypedEvent& event)
	{
		if (s_ModifyTextBufferIndex < _MAX_PATH)
		{
			s_ModifyTextBuffer[s_ModifyTextBufferIndex++] = (char)event.GetKey();
		}
		return false;
	}
	bool InGui::onKeyPressed(KeyPressedEvent& event)
	{
		if (event.IsKeyPressed(KeyCode::KEY_BACKSPACE))
		{
			if (s_ModifyTextBufferIndex > 0)
				s_ModifyTextBufferIndex--;
			s_ModifyTextBuffer[s_ModifyTextBufferIndex] = '0';
		}
		return false;
	}
}