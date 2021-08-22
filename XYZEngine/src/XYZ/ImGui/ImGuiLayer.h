#pragma once

#include "XYZ/Core/Layer.h"

#include "XYZ/Event/ApplicationEvent.h"
#include "XYZ/Event/InputEvent.h"


namespace XYZ {

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;

		void Begin();
		void End();

		void BlockEvents(bool block) { m_BlockEvents = block; }
		void EnableDockspace(bool enable) { m_EnableDockspace = enable; }
		bool GetBlockedEvents() const { return m_BlockEvents; }
		void SetDarkThemeColors();
	private:
		void beginDockspace();
		void endDockspace();

	private:
		bool m_BlockEvents = true;
		bool m_EnableDockspace = true;
		float m_Time = 0.0f;
	};

}