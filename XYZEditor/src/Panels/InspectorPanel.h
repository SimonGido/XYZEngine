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
		bool m_AddComponentOpen = false;

		float m_ScrollOffset = 0.0f;
		float m_ScrollScale = 1.0f;
	};
}