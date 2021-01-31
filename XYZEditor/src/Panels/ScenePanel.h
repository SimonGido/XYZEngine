#pragma once

#include <XYZ.h>

namespace XYZ {

	class ScenePanel
	{
	public:
		ScenePanel(uint32_t panelID);

		void SetContext(Ref<Scene> context);
		void SetSubTexture(Ref<SubTexture> subTexture);

		void OnInGuiRender();
		void OnEvent(Event& event);

	private:
		uint32_t m_PanelID;
		Ref<Scene> m_Context;
		Ref<SubTexture> m_SubTexture;
	};
}