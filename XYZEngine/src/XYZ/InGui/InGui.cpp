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
				for (auto it = winIt->ScrollableOverlayMesh.Quads.begin(); it != winIt->ScrollableOverlayMesh.Quads.end(); ++it)
				{
					Renderer2D::SubmitQuadNotCentered(it->Position, it->Size, it->TexCoord, it->TextureID, it->Color, it->ScissorIndex);
				}
				for (auto& line : winIt->ScrollableOverlayMesh.Lines)
				{
					Renderer2D::SubmitLine(line.P0, line.P1, line.Color);
				}
			}
			Renderer2D::Flush();
			Renderer2D::FlushLines();
		}
		s_Context->FrameData.RestartValues();

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
		else if (dispatcher.Dispatch<MouseScrollEvent>(&InGui::onMouseScroll))
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
		InGuiFrameData& frameData = s_Context->FrameData;

		frameData.LayoutOffset.x = window.Position.x + window.Layout.LeftPadding;
		frameData.LayoutOffset.y += frameData.HighestInRow + window.Layout.TopPadding;
		frameData.HighestInRow = 0.0f;
	}
	void InGui::SetWindowFlags(uint32_t id, uint16_t flags)
	{
		s_Context->Windows[id].Flags = flags;
	}

	void InGui::SetPositionOfNext(const glm::vec2& position)
	{
		InGuiFrameData& frameData = s_Context->FrameData;
		frameData.LayoutOffset = position;
	}

	void InGui::SetTextCenter(InGuiTextCenter center)
	{
		s_Context->FrameData.TextCenter = center;
	}

	const glm::vec2& InGui::GetPositionOfNext()
	{
		InGuiFrameData& frameData = s_Context->FrameData;
		return frameData.LayoutOffset;
	}

	const InGuiWindow& InGui::GetWindow(uint32_t id)
	{
		return s_Context->Windows[id];
	}

	const InGuiContext& InGui::GetContext()
	{
		return *s_Context;
	}


	bool InGui::eraseOutOfBorders(size_t oldQuadCount, const glm::vec2& genSize, const InGuiWindow& window, InGuiMesh& mesh)
	{
		InGuiFrameData& frameData = s_Context->FrameData;
		float xBorder = window.Position.x + window.Size.x - window.Layout.RightPadding;
		float yBorder = window.Position.y + window.Size.y - window.Layout.BottomPadding;
		if (frameData.LayoutOffset.x + genSize.x > xBorder && !frameData.ScrollableActive)
		{
			if (frameData.EraseOutOfLine)
			{
				frameData.LayoutOffset.x += genSize.x + window.Layout.SpacingX;
				mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
				return true;
			}
			else if (!(frameData.LayoutOffset.y + genSize.y > yBorder))
			{
				float oldX = mesh.Quads[oldQuadCount].Position.x;
				mesh.Quads[oldQuadCount].Position.x = window.Position.x + window.Layout.LeftPadding;
				mesh.Quads[oldQuadCount].Position.y += frameData.HighestInRow + window.Layout.TopPadding;
				for (size_t i = oldQuadCount + 1; i < mesh.Quads.size(); ++i)
				{
					float diff = mesh.Quads[i].Position.x - oldX;
					oldX = mesh.Quads[i].Position.x;

					mesh.Quads[i].Position.x = mesh.Quads[i - 1].Position.x + diff;
					mesh.Quads[i].Position.y += frameData.HighestInRow + window.Layout.TopPadding;
				}
				Separator();
				frameData.HighestInRow = genSize.y;
			}
			else
			{
				mesh.Quads.erase(mesh.Quads.begin() + oldQuadCount, mesh.Quads.end());
				return true;
			}
		}
		if (frameData.HighestInRow < genSize.y)
			frameData.HighestInRow = genSize.y;
	
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
		InGuiFrameData& frameData = s_Context->FrameData;

		frameData.HighestInRow = 0.0f;
		frameData.LayoutOffset = glm::vec2(
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
		s_Context->FrameData.CurrentOverlayMesh = &window.OverlayMesh;
		return !IS_SET(window.Flags, InGuiWindowFlags::Collapsed);
	}

	bool InGui::ImageWindow(uint32_t id, const char* name, const glm::vec2& position, const glm::vec2& size, Ref<SubTexture> subTexture)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID == InGuiFrameData::NullID, "Missing end call");
		s_Context->FrameData.ActiveWindowID = id;
		if (!subTexture.Raw()) 
			return false;

		InGuiWindow& window = getInitializedWindow(id, position, size);
		InGuiFrameData& frameData = s_Context->FrameData;

		frameData.HighestInRow = 0.0f;
		frameData.LayoutOffset = glm::vec2(
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
		s_Context->FrameData.CurrentOverlayMesh = &window.OverlayMesh;
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
		InGuiFrameData& frameData = s_Context->FrameData;
		InGuiMesh& mesh = *frameData.CurrentMesh;

		size_t oldQuadCount = mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		glm::vec2 size = {
			window.Size.x - window.Layout.RightPadding - window.Layout.LeftPadding, 
			InGuiWindow::PanelHeight 
		};
		glm::vec2 buttonSize = glm::vec2(InGuiWindow::PanelHeight, InGuiWindow::PanelHeight);
		glm::vec2 pos = { window.Position.x + window.Layout.LeftPadding , frameData.LayoutOffset.y };
		uint32_t subTextureIndex = InGuiRenderData::RIGHT_ARROW;
		if (open) subTextureIndex = InGuiRenderData::DOWN_ARROW;

		
		InGuiFactory::GenerateQuad(mesh, color, size, pos, 
			s_Context->RenderData, InGuiRenderData::BUTTON, s_Context->FrameData.Scissors.size() - 1
		);
		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(name, window, mesh, color, buttonSize, pos, 
			s_Context->RenderData, subTextureIndex, s_Context->FrameData.Scissors.size() - 1, frameData.EraseOutOfLine);


		if (eraseOutOfBorders(oldQuadCount, buttonSize, window, mesh)) { return false; }
		if (open) Separator();

		if (!frameData.ActiveWidgets)
			return open;

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
				
		return open;
	}

	void InGui::BeginScrollableArea(const glm::vec2& size, float& offset, float scale, float scrollSpeed)
	{
		XYZ_ASSERT(!s_Context->FrameData.ScrollableActive, "Missing end scrollable area");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		InGuiFrameData& frameData = s_Context->FrameData;

		auto [wWidth, wHeight] = Input::GetWindowSize();
		float positionX = frameData.LayoutOffset.x;
		float positionY = wHeight - frameData.LayoutOffset.y - size.y;
		s_Context->FrameData.Scissors.push_back(
			{ positionX, positionY, size.x, size.y }
		);

		InGuiFactory::GenerateFrame(window.ScrollableMesh, frameData.LayoutOffset, size, s_Context->RenderData);
		bool activeWidgets = false;
		if (Collide(frameData.LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			activeWidgets = true;
			offset -= frameData.ScrollOffset * scrollSpeed;
		}
		//////////////////// SLIDER LOGIC ///////////////////////
		frameData.LayoutOffset.x += (size.x + window.Layout.LeftPadding);
		float val = offset / scale;
		InGui::SliderVertical("", glm::vec2(25.0f, size.y), val);
		offset = val * scale;
		////////////////////////////////////////////////////////
		frameData.LayoutOffset.x = positionX + window.Layout.LeftPadding;
		frameData.HighestInRow = 0.0f;


		frameData.ActiveWidgets = activeWidgets;
		frameData.ScrollableHeight = frameData.LayoutOffset.y + size.y;
		frameData.LayoutOffset.y -= offset - window.Layout.SpacingY;
		frameData.ScrollableActive = true;
		frameData.CurrentMesh = &window.ScrollableMesh;
		frameData.CurrentOverlayMesh = &window.ScrollableOverlayMesh;
	}

	void InGui::EndScrollableArea()
	{
		XYZ_ASSERT(s_Context->FrameData.ScrollableActive, "Missing begin scrollable area");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		InGuiFrameData& frameData = s_Context->FrameData;

		frameData.CurrentMesh = &window.Mesh;
		frameData.CurrentOverlayMesh = &window.OverlayMesh;
		frameData.ScrollableActive = false;
		frameData.LayoutOffset.y = frameData.ScrollableHeight + window.Layout.SpacingY;
		frameData.ActiveWidgets = true;
	}

	uint8_t InGui::PushNode(const char* name, const glm::vec2& size, bool& open, bool highlight)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		InGuiFrameData& frameData = s_Context->FrameData;
		InGuiMesh& mesh = *frameData.CurrentMesh;

		uint32_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		if (highlight) color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];

		glm::vec2 pos = frameData.LayoutOffset;
		uint32_t subTextureIndex = InGuiRenderData::RIGHT_ARROW;
		if (open) subTextureIndex = InGuiRenderData::DOWN_ARROW;

		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(name, window, mesh, color, size, pos, 
			s_Context->RenderData, subTextureIndex, s_Context->FrameData.Scissors.size() - 1, frameData.EraseOutOfLine
		);
		if (eraseOutOfBorders(oldQuadCount, genSize, window, mesh)) { return false; }
		if (!frameData.ActiveWidgets) return 0;

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
		frameData.LayoutOffset.y += genSize.y;
		return returnType;
	}

	uint8_t InGui::Dropdown(const char* name, const glm::vec2& size, bool& open)
	{
		InGuiFrameData& frameData = s_Context->FrameData;
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		XYZ_ASSERT(!frameData.DropdownItemCount, "Missing end dropdown");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		InGuiMesh& mesh = *s_Context->FrameData.CurrentOverlayMesh;
		
		uint32_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		glm::vec2 pos = frameData.LayoutOffset;
	

		glm::vec2 genSize = InGuiFactory::GenerateQuadWithTextCentered(
			name, s_Context->FrameData.TextCenter,
			window, mesh, color,
			size, pos, s_Context->RenderData, InGuiRenderData::BUTTON, 
			s_Context->FrameData.Scissors.size() - 1
		);
		

		if (eraseOutOfBorders(oldQuadCount, size, window, mesh)) { return false; }
		
		frameData.DropdownSize = size;
		frameData.LayoutOffset.y += size.y;
		frameData.DropdownItemCount++;
		
		if (!frameData.ActiveWidgets) return 0;

		if (Collide(pos, size, s_Context->FrameData.MousePosition))
		{
			mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
			{
				open = !open;
				returnType |= InGuiReturnType::Clicked;
			}
			for (size_t i = oldQuadCount + 1; i < mesh.Quads.size(); ++i)
				mesh.Quads[i].Color = s_Context->RenderData.Color[InGuiRenderData::SELECT_COLOR];
			returnType |= InGuiReturnType::Hoovered;
		}	
		return returnType;
	}

	void InGui::EndDropdown()
	{
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		InGuiFrameData& frameData = s_Context->FrameData;
		frameData.LayoutOffset.y -= frameData.DropdownSize.y * frameData.DropdownItemCount;
		frameData.LayoutOffset.x += window.Layout.SpacingX + frameData.DropdownSize.x;
		frameData.DropdownItemCount = 0;
	}

	uint8_t InGui::DropdownItem(const char* name)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		InGuiFrameData& frameData = s_Context->FrameData;
		InGuiMesh& mesh = *frameData.CurrentOverlayMesh;

		uint32_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		glm::vec2 pos = frameData.LayoutOffset;


		glm::vec2 genSize = InGuiFactory::GenerateQuadWithTextCentered(
			name, s_Context->FrameData.TextCenter,
			window, mesh, color,
			frameData.DropdownSize, pos, s_Context->RenderData, InGuiRenderData::BUTTON,
			s_Context->FrameData.Scissors.size() - 1
		);

		if (eraseOutOfBorders(oldQuadCount, frameData.DropdownSize, window, mesh)) { return false; }
		
		frameData.LayoutOffset.y += frameData.DropdownSize.y;
		frameData.DropdownItemCount++;

		if (!frameData.ActiveWidgets) return 0;

		if (Collide(pos, frameData.DropdownSize, s_Context->FrameData.MousePosition))
		{
			mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
			{
				returnType |= InGuiReturnType::Clicked;
			}
			for (size_t i = oldQuadCount + 1; i < mesh.Quads.size(); ++i)
				mesh.Quads[i].Color = s_Context->RenderData.Color[InGuiRenderData::SELECT_COLOR];
			returnType |= InGuiReturnType::Hoovered;
		}


		return returnType;
	}

	void InGui::BeginChildren()
	{
		InGuiFrameData& frameData = s_Context->FrameData;
		frameData.LayoutOffset.x += InGuiWindow::PanelHeight;
	}
	void InGui::EndChildren()
	{
		InGuiFrameData& frameData = s_Context->FrameData;
		frameData.LayoutOffset.x -= InGuiWindow::PanelHeight;
	}
	uint8_t InGui::Button(const char* name, const glm::vec2& size)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		InGuiFrameData& frameData = s_Context->FrameData;
		InGuiMesh& mesh = *frameData.CurrentMesh;

		uint8_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(
			name, window, mesh, color, size, frameData.LayoutOffset, 
			s_Context->RenderData, InGuiRenderData::BUTTON, s_Context->FrameData.Scissors.size() - 1, frameData.EraseOutOfLine
		);
				
		if (eraseOutOfBorders(oldQuadCount, genSize, window, mesh))
			return returnType;
		if (!frameData.ActiveWidgets)
		{
			frameData.LayoutOffset.x += genSize.x + window.Layout.SpacingX;
			return 0;
		}

		if (Collide(frameData.LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
				returnType |= InGuiReturnType::Clicked;
		}

		frameData.LayoutOffset.x += genSize.x + window.Layout.SpacingX;
		return returnType;
	}

	uint8_t InGui::Checkbox(const char* name, const glm::vec2& size, bool& val)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		InGuiFrameData& frameData = s_Context->FrameData;
		InGuiMesh& mesh = *frameData.CurrentMesh;

		uint8_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		uint32_t subTextureIndex = InGuiRenderData::CHECKBOX_UNCHECKED;
		if (val) subTextureIndex = InGuiRenderData::CHECKBOX_CHECKED;
			
		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(
			name, window, mesh, color, size, frameData.LayoutOffset, 
			s_Context->RenderData, subTextureIndex, s_Context->FrameData.Scissors.size() - 1, frameData.EraseOutOfLine
		);

		if (eraseOutOfBorders(oldQuadCount, genSize, window, mesh))
			return returnType;
		if (!frameData.ActiveWidgets)
		{
			frameData.LayoutOffset.x += genSize.x + window.Layout.SpacingX;
			return 0;
		}

		if (Collide(frameData.LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
			{
				returnType |= InGuiReturnType::Clicked;
				val = !val;
			}
		}
		frameData.LayoutOffset.x += genSize.x + window.Layout.SpacingX;
		return returnType;
	}

	uint8_t InGui::Slider(const char* name, const glm::vec2& size, float& val)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		InGuiFrameData& frameData = s_Context->FrameData;
		InGuiMesh& mesh = *frameData.CurrentMesh;

		// Clamping
		if (val > 0.99f) val = 1.0f;
		else if (val < 0.01f) val = 0.0f;

		int ret = snprintf(frameData.TextBuffer, sizeof(frameData.TextBuffer), "%f", val);
		if (ret < 0) val = 0.0f;

		uint8_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(name, window, mesh, color, size, frameData.LayoutOffset, 
			s_Context->RenderData, InGuiRenderData::SLIDER, s_Context->FrameData.Scissors.size(), frameData.EraseOutOfLine);
		glm::vec2 handleSize = glm::vec2(size.y, size.y);
		glm::vec2 handlePosition = frameData.LayoutOffset + glm::vec2((size.x - size.y) * val, 0.0f);
		InGuiFactory::GenerateQuadWithText(nullptr, window, mesh, color, handleSize, handlePosition, 
			s_Context->RenderData, InGuiRenderData::SLIDER_HANDLE, s_Context->FrameData.Scissors.size() - 1, frameData.EraseOutOfLine
		);
		InGuiFactory::GenerateTextCenteredMiddle(frameData.TextBuffer, window, mesh, frameData.LayoutOffset, size,
			s_Context->RenderData, 1000, s_Context->FrameData.Scissors.size() - 1
		);

		if (eraseOutOfBorders(oldQuadCount, genSize, window, mesh))
			return returnType;
		if (!frameData.ActiveWidgets)
		{
			frameData.LayoutOffset.x += genSize.x + window.Layout.SpacingX;
			return 0;
		}

		if (Collide(frameData.LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (IS_SET(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
			{
				val = (s_Context->FrameData.MousePosition.x - frameData.LayoutOffset.x) / size.x;
				returnType |= InGuiReturnType::Clicked;
			}
		}

		frameData.LayoutOffset.x += genSize.x + window.Layout.SpacingX;
		return returnType;
	}
	uint8_t InGui::SliderVertical(const char* name, const glm::vec2& size, float& val)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		InGuiFrameData& frameData = s_Context->FrameData;
		InGuiMesh& mesh = *frameData.CurrentMesh;

		// Clamping
		if (val > 1.0f - FLT_MIN) val = 1.0f;
		else if (val < FLT_MIN) val = 0.0f;


		uint8_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(name, window, mesh, color, size, frameData.LayoutOffset, 
			s_Context->RenderData, InGuiRenderData::SLIDER, s_Context->FrameData.Scissors.size(), frameData.EraseOutOfLine);
		glm::vec2 handleSize = glm::vec2(size.x, size.x);
		glm::vec2 handlePosition = frameData.LayoutOffset + glm::vec2(0.0f, (size.y - size.x) * val);
		
		InGuiFactory::GenerateQuadWithText(nullptr, window, mesh, color, handleSize, handlePosition,
			s_Context->RenderData, InGuiRenderData::SLIDER_HANDLE, s_Context->FrameData.Scissors.size() - 1, frameData.EraseOutOfLine
		);

		if (eraseOutOfBorders(oldQuadCount, genSize, window, mesh))
			return returnType;
		if (!frameData.ActiveWidgets)
		{
			frameData.LayoutOffset.x += genSize.x + window.Layout.SpacingX;
			return 0;
		}

		if (Collide(frameData.LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (IS_SET(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
			{
				val = (s_Context->FrameData.MousePosition.y - frameData.LayoutOffset.y) / size.y;
				returnType |= InGuiReturnType::Clicked;
			}
		}

		frameData.LayoutOffset.x += genSize.x + window.Layout.SpacingX;
		return returnType;
	}
	uint8_t InGui::Text(const char* text)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		InGuiFrameData& frameData = s_Context->FrameData;
		InGuiMesh& mesh = *frameData.CurrentMesh;

		uint8_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
		glm::vec2 size = glm::vec2(500.0f);
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		

		glm::vec2 genPos = frameData.LayoutOffset + glm::vec2(0.0f, s_Context->RenderData.Font->GetLineHeight());
		glm::vec2 genSize = InGuiFactory::GenerateText(text, mesh, color, genPos, size, 
			s_Context->RenderData, s_Context->FrameData.Scissors.size() - 1
		);

		if (eraseOutOfBorders(oldQuadCount, genSize, window, mesh))
			return returnType;
		if (!frameData.ActiveWidgets)
		{
			frameData.LayoutOffset.x += genSize.x + window.Layout.SpacingX;
			return 0;
		}

		if (Collide(frameData.LayoutOffset, genSize, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			for (size_t i = oldQuadCount; i < mesh.Quads.size(); ++i)
				mesh.Quads[i].Color = s_Context->RenderData.Color[InGuiRenderData::SELECT_COLOR];
			
			if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
				returnType |= InGuiReturnType::Clicked;
		}

		frameData.LayoutOffset.x += genSize.x + window.Layout.SpacingX;
		return returnType;
	}

	uint8_t InGui::Float(const char* name, const glm::vec2& size, float& val)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		InGuiFrameData& frameData = s_Context->FrameData;
		InGuiMesh& mesh = *frameData.CurrentMesh;

		uint8_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		uint32_t maxCharacters = _MAX_PATH;
		char* buffer = frameData.TextBuffer; // By default text buffer

		if (frameData.InputIndex == frameData.HandleInput.size())
			frameData.HandleInput.push_back(false);

		// If input is about to be handled than use buffer for modifying values
		bool handleInput = frameData.HandleInput[frameData.InputIndex];
		if (handleInput)
		{
			color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			buffer = frameData.ModifyTextBuffer;

			frameData.ModifyTextBuffer[frameData.ModifyTextIndex] = '.';
			val = (float)atof(frameData.ModifyTextBuffer);
			maxCharacters = frameData.ModifyTextIndex;
			returnType |= InGuiReturnType::Modified;
		}
		else
		{
			// Otherwise just parse value as string to text buffer
			int ret = snprintf(buffer, sizeof(buffer), "%f", val);
			if (ret < 0) val = 0.0f;				
		}


		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(name, window,mesh, color, size, frameData.LayoutOffset, 
			s_Context->RenderData, InGuiRenderData::BUTTON, s_Context->FrameData.Scissors.size() - 1, frameData.EraseOutOfLine
		);
		InGuiFactory::GenerateTextCenteredMiddle(buffer, window, mesh, frameData.LayoutOffset, size,
			s_Context->RenderData, maxCharacters, s_Context->FrameData.Scissors.size() - 1
		);
		
		if (eraseOutOfBorders(oldQuadCount, genSize, window, mesh))
			return returnType;
		if (!frameData.ActiveWidgets)
		{
			frameData.LayoutOffset.x += genSize.x + window.Layout.SpacingX;
			return 0;
		}

		if (Collide(frameData.LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
			{
				returnType |= InGuiReturnType::Clicked;
				frameData.HandleInput[frameData.InputIndex] = true;
				if (frameData.HandleInput[frameData.InputIndex])
				{
					// This is called once when bit is fliped to true
					int ret = snprintf(frameData.TextBuffer, sizeof(frameData.TextBuffer), "%f", val);
					if (ret < 0) val = 0.0f;

					frameData.ModifyTextIndex = FindNumCharacters(frameData.TextBuffer, size.x, s_Context->RenderData.Font);
					memcpy(frameData.ModifyTextBuffer, frameData.TextBuffer, frameData.ModifyTextIndex);
				}
				for (uint32_t i = 0; i < s_Context->FrameData.HandleInput.size(); ++i)
				{
					if (i != frameData.InputIndex)
						frameData.HandleInput[i] = false;
				}
			}
		}
	
		frameData.InputIndex++;
		frameData.LayoutOffset.x += genSize.x + window.Layout.SpacingX;
		return returnType;
	}
	uint8_t InGui::UInt(const char* name, const glm::vec2& size, uint32_t& val)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		InGuiFrameData& frameData = s_Context->FrameData;
		InGuiMesh& mesh = *s_Context->FrameData.CurrentMesh;

		uint8_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		uint32_t maxCharacters = _MAX_PATH;
		char* buffer = frameData.TextBuffer; // By default text buffer

		if (frameData.InputIndex == frameData.HandleInput.size())
			frameData.HandleInput.push_back(false);

		// If input is about to be handled than use buffer for modifying values
		bool handleInput = frameData.HandleInput[frameData.InputIndex];
		if (handleInput)
		{
			color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			buffer = frameData.ModifyTextBuffer;

			val = (float)atoi(frameData.ModifyTextBuffer);
			maxCharacters = frameData.ModifyTextIndex;
			returnType |= InGuiReturnType::Modified;
		}
		else
		{
			// Otherwise just parse value as string to text buffer
			int ret = snprintf(buffer, sizeof(buffer), "%u", val);
			if (ret < 0) val = 0.0f;
		}


		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(name, window, mesh, color, size, frameData.LayoutOffset,
			s_Context->RenderData, InGuiRenderData::BUTTON, s_Context->FrameData.Scissors.size() - 1, frameData.EraseOutOfLine
		);
		InGuiFactory::GenerateTextCenteredMiddle(buffer, window, mesh, frameData.LayoutOffset, size,
			s_Context->RenderData, maxCharacters, s_Context->FrameData.Scissors.size() - 1
		);

		if (eraseOutOfBorders(oldQuadCount, genSize, window, mesh))
			return returnType;
		if (!frameData.ActiveWidgets)
		{
			frameData.LayoutOffset.x += genSize.x + window.Layout.SpacingX;
			return 0;
		}

		if (Collide(frameData.LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
			{
				returnType |= InGuiReturnType::Clicked;
				frameData.HandleInput[frameData.InputIndex] = true;
				if (frameData.HandleInput[frameData.InputIndex])
				{
					// This is called once when bit is fliped to true
					int ret = snprintf(frameData.TextBuffer, sizeof(frameData.TextBuffer), "%u", val);
					if (ret < 0) val = 0.0f;

					frameData.ModifyTextIndex = FindNumCharacters(frameData.TextBuffer, size.x, s_Context->RenderData.Font);
					memcpy(frameData.ModifyTextBuffer, frameData.TextBuffer, frameData.ModifyTextIndex);
				}
				for (uint32_t i = 0; i < s_Context->FrameData.HandleInput.size(); ++i)
				{
					if (i != frameData.InputIndex)
						frameData.HandleInput[i] = false;
				}
			}
		}

		frameData.InputIndex++;
		frameData.LayoutOffset.x += genSize.x + window.Layout.SpacingX;
		return returnType;
	}

	uint8_t InGui::String(const char* name, const glm::vec2& size, std::string& val)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		InGuiFrameData& frameData = s_Context->FrameData;
		InGuiMesh& mesh = *s_Context->FrameData.CurrentMesh;

		uint8_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		uint32_t maxCharacters = _MAX_PATH;
		char* buffer = frameData.TextBuffer; // By default text buffer

		if (frameData.InputIndex == frameData.HandleInput.size())
			frameData.HandleInput.push_back(false);

		// If input is about to be handled than use buffer for modifying values
		bool handleInput = frameData.HandleInput[frameData.InputIndex];
		if (handleInput)
		{
			color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			buffer = frameData.ModifyTextBuffer;

			val = frameData.ModifyTextBuffer;
			maxCharacters = frameData.ModifyTextIndex;
			returnType |= InGuiReturnType::Modified;
		}
		else
		{
			// Otherwise just parse value as string to text buffer
			int ret = snprintf(buffer, sizeof(buffer), "%s", val.c_str());
			if (ret < 0) val = 0.0f;
		}


		glm::vec2 genSize = InGuiFactory::GenerateQuadWithText(name, window, mesh, color, size, 
			frameData.LayoutOffset, s_Context->RenderData, InGuiRenderData::BUTTON, s_Context->FrameData.Scissors.size() - 1, frameData.EraseOutOfLine
		);
		InGuiFactory::GenerateTextCenteredMiddle(buffer, window, mesh, frameData.LayoutOffset, size,
			s_Context->RenderData, maxCharacters, s_Context->FrameData.Scissors.size() - 1
		);

		if (eraseOutOfBorders(oldQuadCount, genSize, window, mesh))
			return returnType;
		if (!frameData.ActiveWidgets)
		{
			frameData.LayoutOffset.x += genSize.x + window.Layout.SpacingX;
			return 0;
		}

		if (Collide(frameData.LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
			{
				returnType |= InGuiReturnType::Clicked;
				frameData.HandleInput[frameData.InputIndex] = true;
				if (frameData.HandleInput[frameData.InputIndex])
				{
					// This is called once when bit is fliped to true
					int ret = snprintf(frameData.TextBuffer, sizeof(frameData.TextBuffer), "%s", val.c_str());
					frameData.ModifyTextIndex = FindNumCharacters(frameData.TextBuffer, size.x, s_Context->RenderData.Font);
					memcpy(frameData.ModifyTextBuffer, frameData.TextBuffer, frameData.ModifyTextIndex);
				}
				for (uint32_t i = 0; i < s_Context->FrameData.HandleInput.size(); ++i)
				{
					if (i != frameData.InputIndex)
						frameData.HandleInput[i] = false;
				}
			}
		}

		frameData.InputIndex++;
		frameData.LayoutOffset.x += genSize.x + window.Layout.SpacingX;
		return returnType;
	}

	uint8_t InGui::Image(const glm::vec2& size, Ref<SubTexture> subTexture)
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		InGuiWindow& window = s_Context->Windows[s_Context->FrameData.ActiveWindowID];
		InGuiFrameData& frameData = s_Context->FrameData;
		InGuiMesh& mesh = *frameData.CurrentMesh;

		uint8_t returnType = 0;
		size_t oldQuadCount = mesh.Quads.size();
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		InGuiFactory::GenerateQuad(mesh, color, size, frameData.LayoutOffset, subTexture, 
			Renderer2D::SetTexture(subTexture->GetTexture()), s_Context->FrameData.Scissors.size() - 1
		);
		s_Context->FrameData.CustomTextures.push_back(subTexture->GetTexture());

		if (eraseOutOfBorders(oldQuadCount, size, window, mesh))
			return returnType;
		if (!frameData.ActiveWidgets)
		{
			frameData.LayoutOffset.x += size.x + window.Layout.SpacingX;
			return 0;
		}

		if (Collide(frameData.LayoutOffset, size, s_Context->FrameData.MousePosition))
		{
			returnType |= InGuiReturnType::Hoovered;
			mesh.Quads[oldQuadCount].Color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (TurnOffFlag<uint16_t>(s_Context->FrameData.Flags, InGuiInputFlags::LeftClicked))
				returnType |= InGuiReturnType::Clicked;
		}

		frameData.LayoutOffset.x += size.x + window.Layout.SpacingX;
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
				return first.Flags < second.Flags;
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

	bool InGui::onMouseScroll(MouseScrollEvent& event)
	{
		for (auto& window : s_Context->Windows)
		{
			if (IS_SET(window.Flags, InGuiWindowFlags::Initialized)
				&& Collide(window.Position, window.Size, s_Context->FrameData.MousePosition))
			{
				s_Context->FrameData.ScrollOffset = event.GetOffsetY();
				return false;
			}
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
		InGuiFrameData& frameData = s_Context->FrameData;
		if (frameData.ModifyTextIndex < _MAX_PATH)
		{
			frameData.ModifyTextBuffer[frameData.ModifyTextIndex++] = (char)event.GetKey();
		}
		return false;
	}
	bool InGui::onKeyPressed(KeyPressedEvent& event)
	{
		InGuiFrameData& frameData = s_Context->FrameData;
		if (event.IsKeyPressed(KeyCode::KEY_BACKSPACE))
		{
			if (frameData.ModifyTextIndex > 0)
				frameData.ModifyTextIndex--;
			frameData.ModifyTextBuffer[frameData.ModifyTextIndex] = '\0';
		}
		return false;
	}
}