#pragma once


#include <XYZ.h>



namespace XYZ {
	namespace PanelID {
		enum PanelID
		{
			Scene,
			TestPanel,
			InspectorPanel,
			GraphPanel,
			SceneHierarchy,
			Test,
			SpriteEditor,
			ProjectBrowser,
			DeletePanel
		};
	}


	class Panel
	{
	public:
		Panel(uint32_t id);

		virtual void OnInGuiRender() {};
		virtual void OnUpdate(Timestep ts) {};
		virtual void OnEvent(Event& event) {};
		

	protected:
		enum
		{
			FOLDER = InGuiRenderConfiguration::DOCKSPACE + 1,
			SPRITE,
			TEXTURE,
			MATERIAL,
			SHADER,
			LOGO,
			PLAY,
			PAUSE
		};

		const uint32_t m_PanelID;
	};
}