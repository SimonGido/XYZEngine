#pragma once
#include "XYZ/Core/Layer.h"

#include "GuiContext.h"

namespace XYZ {

	class GuiLayer : public Layer
	{
	public:
		GuiLayer();
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;

		GuiContext* CreateContext(ECSManager* ecs, const GuiSpecification& specs);

	private:
		std::vector<GuiContext*> m_GuiContexts;
	};
}