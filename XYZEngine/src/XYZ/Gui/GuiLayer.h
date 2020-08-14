#pragma once
#include "XYZ/Core/Layer.h"

#include "XYZ/Event/ApplicationEvent.h"
#include "XYZ/Event/InputEvent.h"


namespace XYZ {

	class GuiLayer : public Layer
	{
	public:
		GuiLayer();
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(float dt) override;
		virtual void OnEvent(Event& event) override;

	private:
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onMouseMove(MouseMovedEvent& event);
		bool onMouseScroll(MouseScrollEvent& event);
		bool onWindowResized(WindowResizeEvent& event);
	
	};
}