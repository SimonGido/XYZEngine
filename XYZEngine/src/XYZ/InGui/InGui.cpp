#include "stdafx.h"
#include "InGui.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Renderer/Renderer2D.h"

#include "InGuiFactory.h"
#include "InGuiDockspace.h"
#include <glm/gtx/transform.hpp>

#include <yaml-cpp/yaml.h>



namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
	return out;
}

namespace XYZ {

	InGuiContext* InGui::s_Context = nullptr;

	static char s_TextBuffer[_MAX_PATH];
	static char s_ModifyTextBuffer[_MAX_PATH];
	static uint32_t s_ModifyTextBufferIndex = 0;

	static float s_HighestInRow = 0.0f;
	static float s_ScrollOffset = 0.0f;
	static bool s_ActiveWidgets = true;
	static glm::vec2 s_LayoutOffset = glm::vec2(0.0f);
	static glm::vec2 s_OldLayoutOffset = glm::vec2(0.0f);

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
		Renderer2D::SetMaterial(s_Context->RenderData.DefaultMaterial);
		InGuiDockspace::beginFrame(s_Context, (s_Context->FrameData.MovedWindowID != InGuiFrameData::NullID));
	}

	void InGui::EndFrame()
	{
		XYZ_ASSERT(s_Context, "InGuiContext is not initialized");
		
		InGuiDockspace::endFrame(s_Context);
		Renderer2D::SetMaterial(s_Context->RenderData.DefaultMaterial);
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
				for (auto it = winIt->OverlayMesh.Quads.begin(); it != winIt->OverlayMesh.Quads.end(); ++it)
				{
					Renderer2D::SubmitQuadNotCentered(it->Position, it->Size, it->TexCoord, it->TextureID, it->Color);
				}
				for (auto& line : winIt->OverlayMesh.Lines)
				{
					Renderer2D::SubmitLine(line.P0, line.P1, line.Color);
				}
			}	
		}

		Renderer2D::Flush();
		Renderer2D::FlushLines();

		if (s_Context->FrameData.Scissors.size())
		{		
			s_Context->RenderData.ScissorBuffer->Update(s_Context->FrameData.Scissors.data(), s_Context->FrameData.Scissors.size() * sizeof(InGuiScissor));
			s_Context->RenderData.ScissorBuffer->BindRange(0, s_Context->FrameData.Scissors.size() * sizeof(InGuiScissor), 0);
			s_Context->RenderData.ScissorMaterial->Set("u_NumberScissors", s_Context->FrameData.Scissors.size());
			Renderer2D::SetMaterial(s_Context->RenderData.ScissorMaterial);
			for (auto& texture : s_Context->FrameData.CustomTextures)
				Renderer2D::SetTexture(texture);


			for (auto winIt = s_Context->Windows.rbegin(); winIt != s_Context->Windows.rend(); ++winIt)
			{
				for (auto it = winIt->ScrollableMesh.Quads.begin(); it != winIt->ScrollableMesh.Quads.end(); ++it)
				{
					Renderer2D::SubmitQuadNotCentered(it->Position, it->Size, it->TexCoord, it->TextureID, it->Color, it->ScissorIndex);
				}
				for (auto& line : winIt->ScrollableMesh.Lines)
				{
					Renderer2D::SubmitLine(line.P0, line.P1, line.Color);
				}
			}
			Renderer2D::Flush();
			Renderer2D::FlushLines();
			s_Context->FrameData.Scissors.clear();
		}
		s_Context->FrameData.CustomTextures.clear();

		Renderer2D::EndScene();
		Renderer::WaitAndRender();

		handleWindowMove();
		handleWindowResize();
	}


	void InGui::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		if (dispatcher.Dispatch<MouseButtonPressEvent>(&InGui::onMouseButtonPress))
		{
		}
		else if (dispatcher.Dispatch<MouseButtonReleaseEvent>(&InGui::onMouseButtonRelease))
		{
		}
		else if (dispatcher.Dispatch<MouseMovedEvent>(&InGui::onMouseMove))
		{
		}
		else if (dispatcher.Dispatch<KeyTypedEvent>(&InGui::onKeyTyped))
		{
		}
		else if (dispatcher.Dispatch<KeyPressedEvent>(&InGui::onKeyPressed))
		{
		}
		else if (dispatcher.Dispatch<WindowResizeEvent>(&InGui::onWindowResize))
		{
		}
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
	void InGui::SetWindowFlags(uint32_t id, uint16_t flags)
	{
		s_Context->Windows[id].Flags = flags;
	}

	void InGui::SetPosition(const glm::vec2& position)
	{
		s_LayoutOffset = position;
	}

	const InGuiWindow& InGui::GetWindow(uint32_t id)
	{
		return s_Context->Windows[id];
	}

	glm::vec2 InGui::GetPosition()
	{
		return s_LayoutOffset;
	}

	bool InGui::eraseOutOfBorders(size_t oldQuadCount, const glm::vec2& genSize, const InGuiWindow& window, InGuiMesh& mesh)
	{
		if (s_LayoutOffset.x + genSize.x >= window.Position.x + window.Size.x 
		 || s_LayoutOffset.y + genSize.y >= window.Position.y + window.Size.y)
		{
			s_LayoutOffset.x += genSize.x + window.Layout.SpacingX;
			mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
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
			s_Context->EventListeners.push_back(id);
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
	
		s_Context->FrameData.CurrentMesh = &window.Mesh;
		
		return !IS_SET(window.Flags, InGuiWindowFlags::Collapsed);
	}

	bool InGui::ImageWindow(uint32_t id, const char* name, const glm::vec2& position, const glm::vec2& size, Ref<SubTexture> subTexture)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID == InGuiFrameData::NullID, "Missing end call");
		s_Context->FrameData.ActiveWindowID = id;
		if (!subTexture.Raw()) 
			return false;

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
		s_Context->FrameData.CustomTextures.push_back(subTexture->GetTexture());
		s_Context->FrameData.CurrentMesh = &window.Mesh;

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
		InGuiMesh& mesh = *s_Context->FrameData.CurrentMesh;

		size_t oldQuadCount = mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		glm::vec2 size = {
			window.Size.x - window.Layout.RightPadding - window.Layout.LeftPadding, 
			InGuiWindow::PanelHeight 
		};
		glm::vec2 buttonSize = glm::vec2(InGuiWindow::PanelHeight, InGuiWindow::PanelHeight);
		glm::vec2 pos = { window.Position.x + window.Layout.LeftPadding , s_LayoutOffset.y };
		uint32_t subTextureIndex = InGuiRenderData::RIGHT_ARROW;
		if (open) subTextureIndex = InGuiRenderData::DOWN_ARROW;

		
		InGuiFactory::GenerateQuad(mesh, color, size, pos, 
			s_Context->RenderData, InGuiRenderData::BUTTON, s_Context->FrameData.Scissors.size() - 1
		);
		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(name, window, mesh, color, buttonSize, pos, 
			s_Context->RenderData, subTextureIndex, s_Context->FrameData.Scissors.size() - 1);


		if (eraseOutOfBorders(oldQuadCount, buttonSize, window, mesh)) { return false; }
		if (!s_ActiveWidgets) return false;

		if (Collide(pos, size, s_Context->FrameData.MousePosition))
		{
			mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
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

	void InGui::BeginScrollableArea(const glm::vec2& size, float& offset, float scale)
	{
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
	
		auto [wWidth, wHeight] = Input::GetWindowSize();
		float positionY = wHeight - s_LayoutOffset.y - size.y;
		s_Context->FrameData.Scissors.push_back(
			{ s_LayoutOffset.x, positionY, size.x, size.y }
		);

		InGuiFactory::GenerateFrame(window.ScrollableMesh, s_LayoutOffset, size, s_Context->RenderData);
		bool activeWidgets = false;
		if (Collide(s_LayoutOffset, size, s_Context->FrameData.MousePosition))
			activeWidgets = true;

		//////////////////// SLIDER LOGIC ///////////////////////
		s_LayoutOffset.x += size.x + window.Layout.LeftPadding;
		float val = offset / scale;
		InGui::SliderVertical("", glm::vec2(25.0f, size.y), val);
		offset = val * scale;
		s_LayoutOffset.x -= size.x + window.Layout.LeftPadding;
		s_HighestInRow = 0.0f;
		////////////////////////////////////////////////////////
	
		s_ActiveWidgets = activeWidgets;
		s_ScrollOffset = s_LayoutOffset.y + size.y;
		s_LayoutOffset.y -= offset - window.Layout.SpacingY;
		s_Context->FrameData.CurrentMesh = &window.ScrollableMesh;
		window.Layout.LeftPadding += window.Layout.LeftPadding; // Padding relative to scrollable area
	}

	void InGui::EndScrollableArea()
	{
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		s_Context->FrameData.CurrentMesh = &window.Mesh;
		s_LayoutOffset.y = s_ScrollOffset + window.Layout.SpacingY;
		s_ActiveWidgets = true;
		window.Layout.LeftPadding -= window.Layout.LeftPadding / 2.0f; // Remove padding relative to scrollable area
	}

	uint8_t InGui::PushNode(const char* name, const glm::vec2& size, bool& open, bool highlight)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		InGuiMesh& mesh = *s_Context->FrameData.CurrentMesh;

		uint32_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		if (highlight) color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];

		glm::vec2 pos = s_LayoutOffset;
		uint32_t subTextureIndex = InGuiRenderData::RIGHT_ARROW;
		if (open) subTextureIndex = InGuiRenderData::DOWN_ARROW;

		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(name, window, mesh, color, size, pos, 
			s_Context->RenderData, subTextureIndex, s_Context->FrameData.Scissors.size() - 1
		);
		if (eraseOutOfBorders(oldQuadCount, genSize, window, mesh)) { return false; }
		if (!s_ActiveWidgets) return 0;

		if (Collide(pos, genSize, s_Context->FrameData.MousePosition))
		{
			if (Collide(pos, size, s_Context->FrameData.MousePosition))
			{
				mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
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
		return returnType;
	}

	uint8_t InGui::Dropdown(const char* name, const glm::vec2& size, bool& open)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];

		uint32_t returnType = 0;
		size_t oldQuadCount = window.OverlayMesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		glm::vec2 pos = s_LayoutOffset;

		glm::vec2 genSize = InGuiFactory::GenerateQuadWithTextLeft(name, window, window.OverlayMesh, 
			color, size, pos, s_Context->RenderData, InGuiRenderData::BUTTON, s_Context->FrameData.Scissors.size() - 1);
		//if (eraseOutOfBorders(oldQuadCount, genSize, window, window.OverlayMesh)) { return false; }
		if (!s_ActiveWidgets) return 0;

		if (Collide(pos, genSize, s_Context->FrameData.MousePosition))
		{
			if (Collide(pos, size, s_Context->FrameData.MousePosition))
			{
				window.OverlayMesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
				if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
				{
					open = !open;
				}
			}
			else // It collides with text
			{
				for (size_t i = oldQuadCount + 1; i < window.OverlayMesh.Quads.size(); ++i)
					window.OverlayMesh.Quads[i].Color = s_Context->RenderData.Color[InGuiRenderData::SELECT_COLOR];
				returnType |= InGuiReturnType::Hoovered;
				if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
				{
					returnType |= InGuiReturnType::Clicked;
				}
			}
		}
		s_LayoutOffset.y += genSize.y;
		return returnType;
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
		InGuiMesh& mesh = *s_Context->FrameData.CurrentMesh;

		uint8_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(
			name, window, mesh, color, size, s_LayoutOffset, 
			s_Context->RenderData, InGuiRenderData::BUTTON, s_Context->FrameData.Scissors.size() - 1
		);
				
		if (eraseOutOfBorders(oldQuadCount, genSize, window, mesh))
			return returnType;
		if (!s_ActiveWidgets)
		{
			s_LayoutOffset.x += genSize.x + window.Layout.SpacingX;
			return 0;
		}

		if (Collide(s_LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
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
		InGuiMesh& mesh = *s_Context->FrameData.CurrentMesh;

		uint8_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		uint32_t subTextureIndex = InGuiRenderData::CHECKBOX_UNCHECKED;
		if (val) subTextureIndex = InGuiRenderData::CHECKBOX_CHECKED;
			
		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(
			name, window, mesh, color, size, s_LayoutOffset, 
			s_Context->RenderData, subTextureIndex, s_Context->FrameData.Scissors.size() - 1
		);

		if (eraseOutOfBorders(oldQuadCount, genSize, window, mesh))
			return returnType;
		if (!s_ActiveWidgets)
		{
			s_LayoutOffset.x += genSize.x + window.Layout.SpacingX;
			return 0;
		}

		if (Collide(s_LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
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
		InGuiMesh& mesh = *s_Context->FrameData.CurrentMesh;

		// Clamping
		if (val > 0.94f) val = 1.0f;
		if (val < 0.06f) val = 0.0f;

		int ret = snprintf(s_TextBuffer, sizeof(s_TextBuffer), "%f", val);
		if (ret < 0)
			val = 0.0f;

		uint8_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(name, window, mesh, color, size, s_LayoutOffset, s_Context->RenderData, InGuiRenderData::SLIDER, s_Context->FrameData.Scissors.size());
		glm::vec2 handleSize = glm::vec2(size.y, size.y);
		glm::vec2 handlePosition = s_LayoutOffset + glm::vec2((size.x - size.y) * val, 0.0f);
		InGuiFactory::GenerateQuadWithText(nullptr, window, mesh, color, handleSize, handlePosition, 
			s_Context->RenderData, InGuiRenderData::SLIDER_HANDLE, s_Context->FrameData.Scissors.size() - 1
		);
		InGuiFactory::GenerateTextCentered(s_TextBuffer, window, mesh, s_LayoutOffset, size,
			s_Context->RenderData, 1000, s_Context->FrameData.Scissors.size() - 1
		);

		if (eraseOutOfBorders(oldQuadCount, genSize, window, mesh))
			return returnType;
		if (!s_ActiveWidgets)
		{
			s_LayoutOffset.x += genSize.x + window.Layout.SpacingX;
			return 0;
		}

		if (Collide(s_LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (IS_SET(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
			{
				val = (s_Context->FrameData.MousePosition.x - s_LayoutOffset.x) / size.x;
				returnType |= InGuiReturnType::Clicked;
			}
		}

		s_LayoutOffset.x += genSize.x + window.Layout.SpacingX;
		return returnType;
	}
	uint8_t InGui::SliderVertical(const char* name, const glm::vec2& size, float& val)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		InGuiMesh& mesh = *s_Context->FrameData.CurrentMesh;

		// Clamping
		if (val > 0.94f) val = 1.0f;
		if (val < 0.06f) val = 0.0f;


		uint8_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(name, window, mesh, color, size, s_LayoutOffset, s_Context->RenderData, InGuiRenderData::SLIDER, s_Context->FrameData.Scissors.size());
		glm::vec2 handleSize = glm::vec2(size.x, size.x);
		glm::vec2 handlePosition = s_LayoutOffset + glm::vec2(0.0f, (size.y - size.x) * val);
		
		InGuiFactory::GenerateQuadWithText(nullptr, window, mesh, color, handleSize, handlePosition,
			s_Context->RenderData, InGuiRenderData::SLIDER_HANDLE, s_Context->FrameData.Scissors.size() - 1
		);

		if (eraseOutOfBorders(oldQuadCount, genSize, window, mesh))
			return returnType;
		if (!s_ActiveWidgets)
		{
			s_LayoutOffset.x += genSize.x + window.Layout.SpacingX;
			return 0;
		}

		if (Collide(s_LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (IS_SET(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
			{
				val = (s_Context->FrameData.MousePosition.y - s_LayoutOffset.y) / size.y;
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
		InGuiMesh& mesh = *s_Context->FrameData.CurrentMesh;

		uint8_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
		glm::vec2 size = glm::vec2(500.0f);
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		

		glm::vec2 genPos = s_LayoutOffset + glm::vec2(0.0f, s_Context->RenderData.Font->GetLineHeight());
		glm::vec2 genSize = InGuiFactory::GenerateText(text, mesh, color, genPos, size, 
			s_Context->RenderData, s_Context->FrameData.Scissors.size() - 1
		);

		if (eraseOutOfBorders(oldQuadCount, genSize, window, mesh))
			return returnType;
		if (!s_ActiveWidgets)
		{
			s_LayoutOffset.x += genSize.x + window.Layout.SpacingX;
			return 0;
		}

		if (Collide(s_LayoutOffset, genSize, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			for (size_t i = oldQuadCount; i < mesh.Quads.size(); ++i)
				mesh.Quads[i].Color = s_Context->RenderData.Color[InGuiRenderData::SELECT_COLOR];
			
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
		InGuiMesh& mesh = *s_Context->FrameData.CurrentMesh;

		uint8_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
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
			returnType |= InGuiReturnType::Modified;
		}
		else
		{
			// Otherwise just parse value as string to text buffer
			int ret = snprintf(buffer, sizeof(buffer), "%f", val);
			if (ret < 0) val = 0.0f;				
		}


		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(name, window,mesh, color, size, s_LayoutOffset, 
			s_Context->RenderData, InGuiRenderData::BUTTON, s_Context->FrameData.Scissors.size() - 1
		);
		InGuiFactory::GenerateTextCentered(buffer, window, mesh, s_LayoutOffset, size, 
			s_Context->RenderData, maxCharacters, s_Context->FrameData.Scissors.size() - 1
		);
		
		if (eraseOutOfBorders(oldQuadCount, genSize, window, mesh))
			return returnType;
		if (!s_ActiveWidgets)
		{
			s_LayoutOffset.x += genSize.x + window.Layout.SpacingX;
			return 0;
		}

		if (Collide(s_LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
			{
				returnType |= InGuiReturnType::Clicked;
				data.HandleInput[data.InputIndex] = true;
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
	uint8_t InGui::UInt(const char* name, const glm::vec2& size, uint32_t& val)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		InGuiFrameData& data = s_Context->FrameData;
		InGuiMesh& mesh = *s_Context->FrameData.CurrentMesh;

		uint8_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
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

			val = (float)atoi(s_ModifyTextBuffer);
			maxCharacters = s_ModifyTextBufferIndex;
			returnType |= InGuiReturnType::Modified;
		}
		else
		{
			// Otherwise just parse value as string to text buffer
			int ret = snprintf(buffer, sizeof(buffer), "%u", val);
			if (ret < 0) val = 0.0f;
		}


		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(name, window, mesh, color, size, s_LayoutOffset,
			s_Context->RenderData, InGuiRenderData::BUTTON, s_Context->FrameData.Scissors.size() - 1
		);
		InGuiFactory::GenerateTextCentered(buffer, window, mesh, s_LayoutOffset, size, 
			s_Context->RenderData, maxCharacters, s_Context->FrameData.Scissors.size() - 1
		);

		if (eraseOutOfBorders(oldQuadCount, genSize, window, mesh))
			return returnType;
		if (!s_ActiveWidgets)
		{
			s_LayoutOffset.x += genSize.x + window.Layout.SpacingX;
			return 0;
		}

		if (Collide(s_LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
			{
				returnType |= InGuiReturnType::Clicked;
				data.HandleInput[data.InputIndex] = true;
				if (data.HandleInput[data.InputIndex])
				{
					// This is called once when bit is fliped to true
					int ret = snprintf(s_TextBuffer, sizeof(s_TextBuffer), "%u", val);
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

	uint8_t InGui::String(const char* name, const glm::vec2& size, std::string& val)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		InGuiFrameData& data = s_Context->FrameData;
		InGuiMesh& mesh = *s_Context->FrameData.CurrentMesh;

		uint8_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
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

			val = s_ModifyTextBuffer;
			maxCharacters = s_ModifyTextBufferIndex;
			returnType |= InGuiReturnType::Modified;
		}
		else
		{
			// Otherwise just parse value as string to text buffer
			int ret = snprintf(buffer, sizeof(buffer), "%s", val.c_str());
			if (ret < 0) val = 0.0f;
		}


		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(name, window, mesh, color, size, 
			s_LayoutOffset, s_Context->RenderData, InGuiRenderData::BUTTON, s_Context->FrameData.Scissors.size() - 1
		);
		InGuiFactory::GenerateTextCentered(buffer, window, mesh, s_LayoutOffset, size, 
			s_Context->RenderData, maxCharacters, s_Context->FrameData.Scissors.size() - 1
		);

		if (eraseOutOfBorders(oldQuadCount, genSize, window, mesh))
			return returnType;
		if (!s_ActiveWidgets)
		{
			s_LayoutOffset.x += genSize.x + window.Layout.SpacingX;
			return 0;
		}

		if (Collide(s_LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
			{
				returnType |= InGuiReturnType::Clicked;
				data.HandleInput[data.InputIndex] = true;
				if (data.HandleInput[data.InputIndex])
				{
					// This is called once when bit is fliped to true
					int ret = snprintf(s_TextBuffer, sizeof(s_TextBuffer), "%s", val.c_str());
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
		InGuiMesh& mesh = *s_Context->FrameData.CurrentMesh;

		uint8_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		InGuiFactory::GenerateQuad(mesh, color, size, s_LayoutOffset, subTexture, 
			Renderer2D::SetTexture(subTexture->GetTexture()), s_Context->FrameData.Scissors.size() - 1
		);
		s_Context->FrameData.CustomTextures.push_back(subTexture->GetTexture());

		if (eraseOutOfBorders(oldQuadCount, size, window, mesh))
			return returnType;
		if (!s_ActiveWidgets)
		{
			s_LayoutOffset.x += size.x + window.Layout.SpacingX;
			return 0;
		}

		if (Collide(s_LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
				returnType |= InGuiReturnType::Clicked;
		}

		s_LayoutOffset.x += size.x + window.Layout.SpacingX;
		return returnType;
	}

	static void SaveRecursiveDockNode(const InGuiDockNode* node, YAML::Emitter& out)
	{
		if (node)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "DockNode" << YAML::Value << node->ID;
			out << YAML::Key << "Position" << YAML::Value << node->Data.Position;
			out << YAML::Key << "Size" << YAML::Value << node->Data.Size;
			out << YAML::Key << "Type" << YAML::Value << ToUnderlying(node->Type);
			out << YAML::Key << "Windows" << YAML::Value << node->Data.Windows;
			if (node->FirstChild)
				out << YAML::Key << "FirstChild" << YAML::Value << node->FirstChild->ID;
			if (node->SecondChild)
				out << YAML::Key << "SecondChild" << YAML::Value << node->SecondChild->ID;
			out << YAML::EndMap; // Window
			SaveRecursiveDockNode(node->FirstChild, out);
			SaveRecursiveDockNode(node->SecondChild, out);
		}
	}


	void InGui::saveLayout()
	{
		YAML::Emitter out;
		std::ofstream fout("ingui.ingui");

		out << YAML::BeginMap;
		out << YAML::Key << "InGui";
		out << YAML::Value << "InGui";

		out << YAML::Key << "Windows";
		out << YAML::Value << YAML::BeginSeq;
		for (auto& win : s_Context->Windows)
		{
			if (IS_SET(win.Flags, InGuiWindowFlags::Initialized))
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Window" << YAML::Value << win.ID;
				out << YAML::Key << "Flags" << YAML::Value << win.Flags;
				out << YAML::Key << "Position" << YAML::Value << win.Position;
				out << YAML::Key << "Size" << YAML::Value << win.Size;
				out << YAML::EndMap; // Window
			}
		}
		out << YAML::EndSeq;
		/////////////////////////
		out << YAML::Key << "Dockspace";
		out << YAML::Value << YAML::BeginSeq;
		SaveRecursiveDockNode(InGuiDockspace::getRoot(), out);
		out << YAML::EndSeq;
		
		out << YAML::EndMap;
		fout << out.c_str();
	}

	void InGui::loadLayout()
	{
		std::ifstream stream("ingui.ingui");
		if (stream.is_open())
		{
			std::stringstream strStream;
			strStream << stream.rdbuf();
			YAML::Node data = YAML::Load(strStream.str());
			auto& windows = data["Windows"];
			if (windows)
			{
				for (auto& it : windows)
				{
					size_t id = it["Window"].as<size_t>();
					if (s_Context->Windows.size() <= id)
						s_Context->Windows.resize(id + 1);

					s_Context->Windows[id].ID = id;
					s_Context->Windows[id].Position = it["Position"].as<glm::vec2>();
					s_Context->Windows[id].Size = it["Size"].as<glm::vec2>();
					s_Context->Windows[id].Flags = it["Flags"].as<uint16_t>();
					s_Context->EventListeners.push_back(id);
				}
			}
			auto& dockspace = data["Dockspace"];
			if (dockspace)
			{
				struct NodeConstructData
				{
					uint32_t FirstChildID;
					uint32_t SecondChildID;
					InGuiDockNode* Node = nullptr;
				};
				std::vector<NodeConstructData> nodes;
				for (auto& it : dockspace)
				{
					size_t id = it["DockNode"].as<uint32_t>();
					if (id >= nodes.size())
						nodes.resize(id + 1);
					nodes[id].Node = new InGuiDockNode(id);
					nodes[id].Node->Data.Position = it["Position"].as<glm::vec2>();
					nodes[id].Node->Data.Size = it["Size"].as<glm::vec2>();
					nodes[id].Node->Data.Windows = it["Windows"].as<std::vector<uint32_t>>();
					nodes[id].Node->Type = static_cast<InGuiSplitType>(it["Type"].as<uint32_t>());
					if (it["FirstChild"])
						nodes[id].FirstChildID = it["FirstChild"].as<uint32_t>();
					if (it["SecondChild"])
						nodes[id].SecondChildID = it["SecondChild"].as<uint32_t>();
				}
				for (auto node : nodes)
				{
					if (node.Node && node.Node->Type != InGuiSplitType::None)
					{
						nodes[node.FirstChildID].Node->Parent = node.Node;
						nodes[node.SecondChildID].Node->Parent = node.Node;
						node.Node->FirstChild = nodes[node.FirstChildID].Node;
						node.Node->SecondChild = nodes[node.SecondChildID].Node;
					}
				}
				if (nodes.size())
				{
					InGuiDockspace::Init(nodes[0].Node);
					auto [width, height] = Input::GetWindowSize();
					InGuiDockspace::SetRootSize({ width, height });
				}
			}
		}
		stream.close();
	}



	bool InGui::onWindowResize(WindowResizeEvent& event)
	{
		InGuiDockspace::SetRootSize({ event.GetWidth(), event.GetHeight() });
		return false;
	}

	bool InGui::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_LEFT))
		{
			std::sort(s_Context->EventListeners.begin(), s_Context->EventListeners.end(), [](uint32_t a, uint32_t b) {
				InGuiWindow& first = s_Context->Windows[a];
				InGuiWindow& second = s_Context->Windows[b];
				return first.Flags > second.Flags;
			});

			const glm::vec2& mousePos = s_Context->FrameData.MousePosition;
			if (InGuiDockspace::onMouseLeftPress(mousePos))
				return true;
			s_Context->FrameData.Flags |= InGuiInputFlags::LeftClicked;
			for (auto& it : s_Context->FrameData.HandleInput) it = false;
				
			for (uint32_t id : s_Context->EventListeners)
			{
				InGuiWindow& window = s_Context->Windows[id];
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
		for (uint32_t id : s_Context->EventListeners)
		{
			InGuiWindow& window = s_Context->Windows[id];
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
			s_ModifyTextBuffer[s_ModifyTextBufferIndex] = '\0';
		}
		return false;
	}
}