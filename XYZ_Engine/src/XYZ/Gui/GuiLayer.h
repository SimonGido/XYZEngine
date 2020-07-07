#pragma once
#include "XYZ/Core/Layer.h"
#include "XYZ/Renderer/OrthoCamera.h"
#include "XYZ/Renderer/RenderSortSystem.h"
#include "XYZ/Event/ApplicationEvent.h"
#include "XYZ/Event/InputEvent.h"

#include "GuiSystem.h"

namespace XYZ {

	class GuiLayer : public Layer
	{
	public:
		GuiLayer();
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(float dt) override;
		virtual void OnEvent(Event& event) override;

		void SetCamera(OrthoCamera* camera) { m_Camera = camera; };
	private:
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onMouseMove(MouseMovedEvent& event);
		bool onMouseScroll(MouseScrollEvent& event);
		bool onWindowResized(WindowResizeEvent& event);

	private:
		Ref<GuiSystem> m_GuiSystem;
		Ref<RenderSortSystem> m_SortSystem;

		// TODO temporary
		OrthoCamera* m_Camera = nullptr;
	};
}