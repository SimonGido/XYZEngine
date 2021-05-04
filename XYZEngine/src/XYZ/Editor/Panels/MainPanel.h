#pragma once
#include "XYZ/Editor/EditorUI.h"

namespace XYZ {
	namespace Editor {
		class MainPanel : public EditorUI
		{
		public:
			MainPanel(const std::string& filepath);
			~MainPanel();

			virtual void OnUpdate(Timestep ts) override;
			virtual void OnReload() override;
			virtual void SetupUI() override;

			void OnEvent(Event& event);
		private:
			float findPerWindowWidth();

		private:
			bUILayout m_ViewLayout;
		};
	}
}