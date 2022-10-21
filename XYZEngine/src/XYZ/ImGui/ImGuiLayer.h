#pragma once

#include "XYZ/Core/Layer.h"

#include "XYZ/Event/ApplicationEvent.h"
#include "XYZ/Event/InputEvent.h"

#include <imgui.h>

namespace XYZ {

	struct ImGuiFontConfig
	{
		std::string    Filepath;
		float		   SizePixels;
		const ImWchar* GlyphRange;
	};

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		virtual ~ImGuiLayer() = default;

		virtual void Begin() = 0;
		virtual void End() = 0;
		virtual void AddFont(const ImGuiFontConfig& config) {}
		virtual bool IsMultiViewport() const { return false; }

		void BlockEvents(bool block) { m_BlockEvents = block; }
		void EnableDockspace(bool enable) { m_EnableDockspace = enable; }
		bool GetBlockedEvents() const { return m_BlockEvents; }
			
		void SetDarkThemeColors();
		void LoadStyle(const std::string& filepath);
		void SaveStyle(const std::string& filepath);

		bool IsSRGB() const { return m_SRGBColorSpace; }

		virtual const std::vector<ImGuiFontConfig>& GetLoadedFonts() const { return std::vector<ImGuiFontConfig>(); }

		static ImGuiLayer* Create();
	
	protected:
		static void beginDockspace();
		static void endDockspace();

		
	protected:
		bool  m_BlockEvents = true;
		bool  m_EnableDockspace = true;
		bool  m_SRGBColorSpace = false;
		float m_Time = 0.0f;
	};

}