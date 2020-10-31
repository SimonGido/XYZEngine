#pragma once
#include "../Event/EditorEvent.h"
#include "../Inspectable/Inspectable.h"
#include "Panel.h"

namespace XYZ {

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