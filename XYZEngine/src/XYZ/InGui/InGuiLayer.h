#pragma once
#include "XYZ/Core/Layer.h"

namespace XYZ {
	class InGuiLayer : public Layer
	{
	public:
		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;

		virtual void OnInGuiRender() override;

		void Begin();
		void End();

		void BlockEvents(bool block) { m_BlockEvents = block; }
	private:
		bool m_BlockEvents = false;
	};
}