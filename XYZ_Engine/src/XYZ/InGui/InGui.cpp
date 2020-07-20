#include "stdafx.h"
#include "InGui.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Renderer/InGuiRenderer.h"
#include "ini.h"

namespace XYZ {

	struct InGuiWindow
	{
		glm::vec2 Position;
		glm::vec2 Size;
		bool Collapsed;
	};

	static InGuiFrameData s_InGuiData;
	static InGuiRenderData s_InGuiRenderData;
	static int32_t s_WindowCount = 0;
	static bool s_Initialized = false;

	static std::unordered_map<std::string, bool> m_CacheStates;
	static std::unordered_map<std::string, InGuiWindow> m_InGuiWindows;

	static glm::vec2 StringToVec2(const std::string& src)
	{
		glm::vec2 val;
		size_t split = src.find(",", 0);

		val.x = std::stof(src.substr(0, split));
		val.y = std::stof(src.substr(split + 1, src.size() - split));

		return val;
	}

	static glm::vec2 MouseToWorld(const glm::vec2& point)
	{
		glm::vec2 offset = { s_InGuiData.WindowSizeX / 2,s_InGuiData.WindowSizeY / 2 };
		return { point.x - offset.x, offset.y - point.y };
	}

	static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
	{
		glm::vec2 offset = { s_InGuiData.WindowSizeX / 2,s_InGuiData.WindowSizeY / 2 };

		return (pos.x + (size.x / 2) >= point.x - offset.x &&
			pos.x - (size.x / 2) <= point.x - offset.x &&
			pos.y + (size.y / 2) >= offset.y - point.y &&
			pos.y - (size.y / 2) <= offset.y - point.y);
	}

	void InGui::Init(const InGuiRenderData& renderData)
	{
		s_InGuiRenderData = renderData;
		s_InGuiData.ModifiedPanel = -1;
		s_Initialized = true;

		mINI::INIFile file("ingui.ini");
		mINI::INIStructure ini;
		if (file.read(ini))
		{
			for (auto& it : ini)
			{
				m_InGuiWindows[it.first].Position = StringToVec2(it.second.get("position"));
				m_InGuiWindows[it.first].Size = StringToVec2(it.second.get("size"));
				m_InGuiWindows[it.first].Collapsed = (bool)atoi(it.second.get("collapsed").c_str());
			}
		}
		else
		{
			file.generate(ini);
		}
	}

	void InGui::Shutdown()
	{
		mINI::INIFile file("ingui.ini");
		mINI::INIStructure ini;
		for (auto& it : m_InGuiWindows)
		{
			std::string pos = std::to_string(it.second.Position.x) + "," + std::to_string(it.second.Position.y);
			std::string size = std::to_string(it.second.Size.x) + "," + std::to_string(it.second.Size.y);
			ini[it.first]["position"] = pos;
			ini[it.first]["size"] = size;
			ini[it.first]["collapsed"] = std::to_string(it.second.Collapsed);
		}

		file.write(ini);
	}

	void InGui::BeginFrame()
	{
		s_InGuiData.MousePosition.x = Input::GetMouseX();
		s_InGuiData.MousePosition.y = Input::GetMouseY();
		s_InGuiData.WindowSizeX = Input::GetWindowSize().first;
		s_InGuiData.WindowSizeY = Input::GetWindowSize().second;

		InGuiRenderer::BeginScene({ {s_InGuiData.WindowSizeX,s_InGuiData.WindowSizeY} });
		InGuiRenderer::SetMaterial(s_InGuiRenderData.Material);
	}
	void InGui::EndFrame()
	{
		InGuiRenderer::Flush();
		s_WindowCount = 0;
		s_InGuiData.ColapsedPanel = false;
	}

	bool InGui::Begin(const std::string& name, const glm::vec2& position, const glm::vec2& size, float panelSize)
	{
		if (!s_Initialized)
			return false;

		glm::vec4 panelColor = { 1,1,1,1 };
		glm::vec4 color = { 1,1,1,1 };
		glm::vec2 offset = { 10,10 };
		std::string copyName = name;

		std::transform(copyName.begin(), copyName.end(), copyName.begin(), ::tolower);

		auto& it = m_InGuiWindows.find(copyName);
		if (it == m_InGuiWindows.end())
		{
			m_InGuiWindows[copyName] = { position,size };
			it = m_InGuiWindows.find(copyName);
		}
		
		glm::vec2 panelPos = { it->second.Position.x, it->second.Position.y + it->second.Size.y / 2 + panelSize / 2 };
		glm::vec2 minButtonPos = { panelPos.x + it->second.Size.x / 2 - panelSize / 2, panelPos.y };
		

		// If collide with panel and right button is down set this window as modified
		if (Collide(panelPos, { size.x,panelSize }, s_InGuiData.MousePosition) 
			&& s_InGuiData.RightMouseButtonDown
			&& s_InGuiData.ModifiedPanel == -1)
		{
			s_InGuiData.ModifiedPanel = s_WindowCount;
		}
		// If this window is modified modify it
		else if (s_InGuiData.ModifiedPanel == s_WindowCount)
		{
			panelColor = s_InGuiRenderData.HooverColor;
			panelPos = MouseToWorld(s_InGuiData.MousePosition);
			minButtonPos = { panelPos.x + it->second.Size.x / 2 - panelSize / 2, panelPos.y };
			it->second.Position = { panelPos.x, panelPos.y - it->second.Size.y / 2 - panelSize / 2 };
		}
		// If hoover over window set as active
		else if (Collide(it->second.Position, it->second.Size, s_InGuiData.MousePosition))
		{
			s_InGuiData.ActivePanel = true;
		}
		// Handle colapsing
		else if (Collide(minButtonPos, { panelSize,panelSize }, s_InGuiData.MousePosition)
			&& s_InGuiData.LeftMouseButtonDown
			&& !s_InGuiData.ActiveWidget)
		{
			it->second.Collapsed = !it->second.Collapsed;
			s_InGuiData.ActiveWidget = true;
		}
		
		s_InGuiData.ColapsedPanel = it->second.Collapsed;
		s_InGuiData.CurrentPanelPosition = it->second.Position;
		
		InGuiRenderer::SubmitUI(panelPos, { it->second.Size.x ,panelSize }, s_InGuiRenderData.SliderSubTexture->GetTexCoords(), s_InGuiRenderData.TextureID, panelColor);
		InGuiRenderer::SubmitCenteredText(name, s_InGuiRenderData.Font, panelPos, { 0.7,0.7 }, s_InGuiRenderData.FontTextureID, s_InGuiRenderData.TextColor);
		InGuiRenderer::SubmitUI(minButtonPos, { panelSize ,panelSize }, s_InGuiRenderData.MinimizeButtonSubTexture->GetTexCoords(), s_InGuiRenderData.TextureID, { 1,1,1,1 });

		if (!it->second.Collapsed)
			InGuiRenderer::SubmitUI(it->second.Position, it->second.Size, s_InGuiRenderData.WindowSubTexture->GetTexCoords(), s_InGuiRenderData.TextureID, color);
		
		s_WindowCount++;
		return false;
	}

	void InGui::End()
	{
		s_InGuiData.CurrentPanelPosition = { 0,0 };
		s_InGuiData.ActivePanel = false;
	}

	bool InGui::Button(const std::string& name, const glm::vec2& position, const glm::vec2& size)
	{
		if (!s_Initialized)
			return false;

		bool pressed = false;
		glm::vec4 color = { 1,1,1,1 };
		glm::vec2 offset = s_InGuiData.CurrentPanelPosition;

		if (s_InGuiData.ActivePanel && !s_InGuiData.ColapsedPanel)
		{
			if (Collide(position + offset, size, s_InGuiData.MousePosition))
			{
				color = s_InGuiRenderData.HooverColor;
				if (!s_InGuiData.ActiveWidget)
				{
					pressed = s_InGuiData.LeftMouseButtonDown;
					s_InGuiData.ActiveWidget = pressed;
				}
			}
		}

		if (!s_InGuiData.ColapsedPanel)
		{
			InGuiRenderer::SubmitUI(position + offset, size, s_InGuiRenderData.ButtonSubTexture->GetTexCoords(), s_InGuiRenderData.TextureID, color);
			InGuiRenderer::SubmitCenteredText(name, s_InGuiRenderData.Font, position + offset, { 0.7,0.7 }, s_InGuiRenderData.FontTextureID, s_InGuiRenderData.TextColor);
		}
		return pressed;
	}

	bool InGui::Checkbox(const std::string& name, const glm::vec2& position, const glm::vec2& size)
	{
		if (!s_Initialized)
			return false;

		glm::vec2 offset = s_InGuiData.CurrentPanelPosition;
		glm::vec4 color = { 1,1,1,1 };
		auto it = m_CacheStates.find(name);
		if (it == m_CacheStates.end())
		{
			m_CacheStates[name] = false;
			it = m_CacheStates.find(name);
		}

		if (s_InGuiData.ActivePanel && !s_InGuiData.ColapsedPanel)
		{
			if (Collide(position + offset, size, s_InGuiData.MousePosition))
			{
				color = s_InGuiRenderData.HooverColor;
				if (s_InGuiData.LeftMouseButtonDown && !s_InGuiData.ActiveWidget)
				{
					it->second = !it->second;
					s_InGuiData.ActiveWidget = true;
				}
			}
		}

		if (!s_InGuiData.ColapsedPanel)
		{
			if (it->second)
				InGuiRenderer::SubmitUI(position + offset, size, s_InGuiRenderData.CheckboxSubTextureChecked->GetTexCoords(), s_InGuiRenderData.TextureID, color);
			else
				InGuiRenderer::SubmitUI(position + offset, size, s_InGuiRenderData.CheckboxSubTextureUnChecked->GetTexCoords(), s_InGuiRenderData.TextureID, color);

			InGuiRenderer::SubmitCenteredText(name, s_InGuiRenderData.Font, { position.x + size.x / 2 + offset.x + 10,position.y + offset.y }, { 0.7,0.7 }, s_InGuiRenderData.FontTextureID, s_InGuiRenderData.TextColor, InGuiRenderer::Middle | InGuiRenderer::Right);
			//InGuiRenderer::SubmitCenteredText(name, s_InGuiRenderData.Font, { position.x + offset.x,position.y + (size.y / 2) + offset.y }, { 0.7,0.7 }, s_InGuiRenderData.FontTextureID, s_InGuiRenderData.TextColor, InGuiRenderer::Middle | InGuiRenderer::Top);
		}
		return it->second;
	}

	bool InGui::Slider(const std::string& name, const glm::vec2& position, const glm::vec2& size, float& value)
	{
		if (!s_Initialized)
			return false;

		glm::vec2 offset = s_InGuiData.CurrentPanelPosition;
		glm::vec4 color = { 1,1,1,1 };
		glm::vec2 handlePos = { position.x - (size.x / 2) + value, position.y };
		glm::vec2 handleSize = { size.y, size.y * 2 };
		bool modified = false;

		if (s_InGuiData.ActivePanel && !s_InGuiData.ColapsedPanel)
		{
			if (Collide(position + offset, size, s_InGuiData.MousePosition))
			{
				color = s_InGuiRenderData.HooverColor;
				modified = s_InGuiData.LeftMouseButtonDown;
				if (modified && !s_InGuiData.ActiveWidget)
				{
					float start = position.x + offset.x - size.x / 2.0f;
					value = MouseToWorld(s_InGuiData.MousePosition).x - start;
				}
			}
		}

		if (!s_InGuiData.ColapsedPanel)
		{
			InGuiRenderer::SubmitUI(position + offset, size, s_InGuiRenderData.SliderSubTexture->GetTexCoords(), s_InGuiRenderData.TextureID, color);
			InGuiRenderer::SubmitUI(handlePos + offset, handleSize, s_InGuiRenderData.SliderHandleSubTexture->GetTexCoords(), s_InGuiRenderData.TextureID, color);

			InGuiRenderer::SubmitCenteredText(name, s_InGuiRenderData.Font, { position.x + size.x / 2 + offset.x + 10,position.y + offset.y }, { 0.7,0.7 }, s_InGuiRenderData.FontTextureID, s_InGuiRenderData.TextColor, InGuiRenderer::Middle | InGuiRenderer::Right);
			//InGuiRenderer::SubmitCenteredText(name, s_InGuiRenderData.Font, { position.x + offset.x,position.y + (size.y / 2) + offset.y }, { 0.7,0.7 }, s_InGuiRenderData.FontTextureID, s_InGuiRenderData.TextColor, InGuiRenderer::Middle | InGuiRenderer::Top);
		}
		return modified;
	}

	bool InGui::Image(const std::string& name, uint32_t rendererID, const glm::vec2& position, const glm::vec2& size)
	{
		if (!s_Initialized)
			return false;

		glm::vec2 offset = s_InGuiData.CurrentPanelPosition;
		glm::vec4 color = { 1,1,1,1 };

		if (s_InGuiData.ActivePanel && !s_InGuiData.ColapsedPanel)
		{
			if (Collide(position + offset, size, s_InGuiData.MousePosition))
			{
				color = s_InGuiRenderData.HooverColor;
			}
		}

		if (!s_InGuiData.ColapsedPanel)
			InGuiRenderer::SubmitUI(rendererID, position + offset, size, { 0,0,1,1 }, color);
		
		return false;
	}

	bool InGui::RenderWindow(const std::string& name, uint32_t rendererID, const glm::vec2& position, const glm::vec2& size, float panelSize)
	{
		if (!s_Initialized)
			return false;

		glm::vec4 color = { 1,1,1,1 };
		glm::vec4 panelColor = { 1,1,1,1 };
		bool hoover = false;
		std::string copyName = name;
		std::transform(copyName.begin(), copyName.end(), copyName.begin(), ::tolower);

		auto& it = m_InGuiWindows.find(copyName);
		if (it == m_InGuiWindows.end())
		{
			m_InGuiWindows[copyName] = { position,size };
			it = m_InGuiWindows.find(copyName);
		}

		glm::vec2 panelPos = { it->second.Position.x, it->second.Position.y + it->second.Size.y / 2 + panelSize / 2 };
		glm::vec2 minButtonPos = { panelPos.x + it->second.Size.x / 2 - panelSize / 2, panelPos.y };
		
		
		// If collide with panel and right button is down set this window as modified
		if (Collide(panelPos, { size.x,panelSize }, s_InGuiData.MousePosition) 
			&& s_InGuiData.RightMouseButtonDown
			&& s_InGuiData.ModifiedPanel == -1)
		{	
			s_InGuiData.ModifiedPanel = s_WindowCount;		
		}
		// If this window is modified modify it
		else if (s_InGuiData.ModifiedPanel == s_WindowCount)
		{
			panelColor = s_InGuiRenderData.HooverColor;
			panelPos = MouseToWorld(s_InGuiData.MousePosition);
			minButtonPos = { panelPos.x + it->second.Size.x / 2 - panelSize / 2, panelPos.y };
			it->second.Position = { panelPos.x, panelPos.y - it->second.Size.y / 2 - panelSize / 2 };
		}
		else if (Collide(it->second.Position, it->second.Size, s_InGuiData.MousePosition))
		{
			color = s_InGuiRenderData.HooverColor;
			hoover = true;
		}
		// Handle colapsing
		else if (Collide(minButtonPos, { panelSize,panelSize }, s_InGuiData.MousePosition)
			&& s_InGuiData.LeftMouseButtonDown
			&& !s_InGuiData.ActiveWidget)
		{
			it->second.Collapsed = !it->second.Collapsed;
			s_InGuiData.ActiveWidget = true;
		}
		

		InGuiRenderer::SubmitUI(panelPos, { size.x,panelSize }, s_InGuiRenderData.SliderSubTexture->GetTexCoords(), s_InGuiRenderData.TextureID, panelColor);
		InGuiRenderer::SubmitCenteredText(name, s_InGuiRenderData.Font, panelPos, { 0.7,0.7 }, s_InGuiRenderData.FontTextureID, s_InGuiRenderData.TextColor, InGuiRenderer::Middle);
		InGuiRenderer::SubmitUI(minButtonPos, { panelSize ,panelSize }, s_InGuiRenderData.MinimizeButtonSubTexture->GetTexCoords(), s_InGuiRenderData.TextureID, { 1,1,1,1 });
		
		if (!it->second.Collapsed)
			InGuiRenderer::SubmitUI(rendererID, it->second.Position, it->second.Size, { 0,0,1,1 }, color);
	
		s_WindowCount++;
		return hoover;
	}

	InGuiFrameData& InGui::GetData()
	{
		return s_InGuiData;
	}

}