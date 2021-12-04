#pragma once

#include "XYZ/Core/Layer.h"

#include "XYZ/Event/ApplicationEvent.h"
#include "XYZ/Event/InputEvent.h"


namespace XYZ {

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		virtual ~ImGuiLayer() = default;

		virtual void Begin() = 0;
		virtual void End() = 0;

		void BlockEvents(bool block) { m_BlockEvents = block; }
		void EnableDockspace(bool enable) { m_EnableDockspace = enable; }
		bool GetBlockedEvents() const { return m_BlockEvents; }
			
		void SetDarkThemeColors();
		void LoadStyle(const std::string& filepath);
		void SaveStyle(const std::string& filepath);

		bool IsSRGB() const { return m_SRGBColorSpace; }

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