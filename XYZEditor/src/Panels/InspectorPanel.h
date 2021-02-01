#pragma once

#include <XYZ.h>

namespace XYZ {

	class InspectorPanel
	{
	public:
		InspectorPanel(uint32_t panelID);
		void SetContext(SceneEntity context);

		void OnInGuiRender();

	private:
		void resizeGroups();

	private:
		uint32_t m_PanelID;
		SceneEntity m_Context;
		size_t m_CurrentSize = 0;
		bool* m_ComponentGroups = nullptr;
	};
}