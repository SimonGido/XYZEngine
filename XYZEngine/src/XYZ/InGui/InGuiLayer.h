#pragma once
#include "XYZ/Core/Layer.h"
#include "XYZ/Renderer/Camera.h"

namespace XYZ {

	class InGuiLayer : public Layer
	{
	public:
		virtual void OnAttach();
		virtual void OnDetach();
		virtual void OnEvent(Event& event);

		void Begin();
		void End();
	
	private:
		Camera m_Camera;
	};
}