#pragma once

#include <XYZ.h>

namespace XYZ {

	class InspectorPanel
	{
	public:
		InspectorPanel(uint32_t panelID);

		void OnInGuiRender();

	private:
		uint32_t m_PanelID;

	};
}