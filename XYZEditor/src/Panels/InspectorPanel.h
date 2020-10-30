#pragma once
#include "../Event/EditorEvent.h"
#include "../Inspectable/Inspectable.h"
#include "Panel.h"

namespace XYZ {

	class InspectorLayout
	{
	public:
		virtual void OnInGuiRender() {};
		virtual void OnUpdate(Timestep ts) {};
		virtual void OnEvent(Event& event) {};
	};

	class InspectorPanel : public Panel
	{
	public:
		InspectorPanel(uint32_t id);
		virtual void OnInGuiRender() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;
		virtual void SetInspectable(Inspectable* inspectable);

	private:
		Inspectable* m_Inspectable = nullptr;

		friend class InspectorLayout;
	};
}