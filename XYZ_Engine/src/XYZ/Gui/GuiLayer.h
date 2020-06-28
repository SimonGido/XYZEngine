#pragma once
#include "XYZ/Core/Layer.h"


namespace XYZ {

	class GuiLayer : public Layer
	{
	public:
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(float dt) override;
		virtual void OnEvent(Event& event) override;
	private:

	};
}