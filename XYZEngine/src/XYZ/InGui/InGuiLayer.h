#pragma once
#include "XYZ/Core/Layer.h"
#include "XYZ/Gui/Font.h"
#include "XYZ/Renderer/Material.h"


#include "XYZ/Event/InputEvent.h"
#include "XYZ/Event/ApplicationEvent.h"

namespace XYZ {
	class InGuiLayer : public Layer
	{
	public:
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(float dt) override;
		virtual void OnEvent(Event& event) override;
		void Begin();
		void End();
	private:
		bool onMouseButtonPress(MouseButtonPressEvent& e);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& e);
		bool onMouseMove(MouseMovedEvent& e);
		bool onWindowResize(WindowResizeEvent& e);

	private:
		Ref<Material> m_Material;
	};

}