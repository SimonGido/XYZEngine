#pragma once
#include "Editor/EditorPanel.h"


namespace XYZ {
	namespace Editor {
		class ImGuiStylePanel : public EditorPanel
		{
		public:
			ImGuiStylePanel(std::string name);
			~ImGuiStylePanel();

			virtual void OnImGuiRender(bool& open) override;

		private:
			bool styleTab() const;
			bool fontTab();

			void handleAddFont();
			void handleLimits() const;

		private:
			bool m_AutoSave = false;
			std::string m_AutoSaveFile;

			std::string m_LoadFontPath;
			float		m_LoadFontSize = 15.0f;
		};
	}
}