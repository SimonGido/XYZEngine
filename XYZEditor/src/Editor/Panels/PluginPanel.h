#pragma once
#include "Editor/EditorPanel.h"

#include "XYZ.h"

namespace XYZ {
	namespace Editor {
		class PluginPanel : public EditorPanel
		{
		public:
			PluginPanel(std::string name);

			virtual void OnImGuiRender(bool& open) override;
			virtual void OnUpdate(Timestep ts) override;
			virtual bool OnEvent(Event& e) override;

			virtual void SetSceneContext(const Ref<Scene>& scene) override;
		
		private:
			PluginLanguage m_CreatePluginLanguage;
			std::string	   m_CreatePluginLanguageString;
			std::string	   m_CreatePluginName;
			std::string	   m_CreatePluginDirectory;
		};
	}
}