#include "stdafx.h"
#include "InGuiCore.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Renderer/InGuiRenderer.h"


namespace XYZ {

	static InGuiFrameData s_InGuiData;
	static InGuiRenderData s_InGuiRenderData;
	static bool s_Initialized = false;

	static std::unordered_map<std::string, bool> m_CacheStates;


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
				pos.y - (size.y / 2) <= offset.y - point.y );
	}

	void InGui::Init(const InGuiRenderData& renderData)
	{
		s_InGuiRenderData = renderData;
		s_Initialized = true;
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
	}

	bool InGui::Button(const std::string& name, const glm::vec2& position, const glm::vec2& size)
	{
		if (!s_Initialized)
			return false;
		

		bool pressed = false;
		glm::vec4 color = { 1,1,1,1 };
		if (Collide(position, size, s_InGuiData.MousePosition))
		{			
			color = s_InGuiRenderData.HooverColor;		
			if (!s_InGuiData.ActiveWidget)
			{
				pressed = s_InGuiData.LeftMouseButtonDown;
				s_InGuiData.ActiveWidget = pressed;
			}		
		}

		InGuiRenderer::SubmitUI(position, size, s_InGuiRenderData.ButtonSubTexture->GetTexCoords(), s_InGuiRenderData.TextureID, color);
		InGuiRenderer::SubmitCenteredText(name, s_InGuiRenderData.Font, position, { 0.7,0.7 }, s_InGuiRenderData.FontTextureID, s_InGuiRenderData.TextColor);
		return pressed;
	}

	bool InGui::Checkbox(const std::string& name, const glm::vec2& position, const glm::vec2& size)
	{
		if (!s_Initialized)
			return false;


		glm::vec4 color = { 1,1,1,1 };
		auto it = m_CacheStates.find(name);
		if (it == m_CacheStates.end())
		{
			m_CacheStates[name] = false;
			it = m_CacheStates.find(name);
		}
		if (Collide(position, size, s_InGuiData.MousePosition))
		{
			color = s_InGuiRenderData.HooverColor;
			if (s_InGuiData.LeftMouseButtonDown && !s_InGuiData.ActiveWidget)
			{
				it->second = !it->second;
				s_InGuiData.ActiveWidget = true;
			}
		}

		if (it->second)
			InGuiRenderer::SubmitUI(position, size, s_InGuiRenderData.CheckboxSubTextureChecked->GetTexCoords(), s_InGuiRenderData.TextureID, color);
		else
			InGuiRenderer::SubmitUI(position, size, s_InGuiRenderData.CheckboxSubTextureUnChecked->GetTexCoords(), s_InGuiRenderData.TextureID, color);
		
		InGuiRenderer::SubmitCenteredText(name, s_InGuiRenderData.Font, { position.x,position.y + (size.y / 2) }, { 0.7,0.7 }, s_InGuiRenderData.FontTextureID, s_InGuiRenderData.TextColor, InGuiRenderer::Middle | InGuiRenderer::Top);
		return it->second;
	}

	bool InGui::Slider(const std::string& name, const glm::vec2& position, const glm::vec2& size, float& value)
	{
		if (!s_Initialized)
			return false;

		glm::vec4 color = { 1,1,1,1 };
		glm::vec2 handlePos = { position.x - (size.x / 2) + value, position.y };
		glm::vec2 handleSize = { size.y, size.y * 2 };
		bool modified = false;
		if (Collide(position, size, s_InGuiData.MousePosition))
		{
			color = s_InGuiRenderData.HooverColor;
			modified = s_InGuiData.LeftMouseButtonDown;
			if (modified && !s_InGuiData.ActiveWidget)
			{
				float start = position.x - size.x / 2.0f;
				value = MouseToWorld(s_InGuiData.MousePosition).x - start;
			}
		}
	
		InGuiRenderer::SubmitUI(position, size, s_InGuiRenderData.SliderSubTexture->GetTexCoords(), s_InGuiRenderData.TextureID, color);
		InGuiRenderer::SubmitUI(handlePos, handleSize, s_InGuiRenderData.SliderHandleSubTexture->GetTexCoords(), s_InGuiRenderData.TextureID, color);
		InGuiRenderer::SubmitCenteredText(name, s_InGuiRenderData.Font, { position.x,position.y + (size.y / 2) }, { 0.7,0.7 }, s_InGuiRenderData.FontTextureID, s_InGuiRenderData.TextColor, InGuiRenderer::Middle | InGuiRenderer::Top);
		
		return modified;
	}

	InGuiFrameData& InGui::GetData()
	{
		return s_InGuiData;
	}

}