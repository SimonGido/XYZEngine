#pragma once
#include "XYZ/Editor/EditorPanel.h"



namespace XYZ {
	namespace Editor {
		class ImGuiStylePanel : public EditorPanel
		{
		public:
			ImGuiStylePanel(std::string name);
			~ImGuiStylePanel();

			virtual void OnImGuiRender(bool& open) override;

		private:
			void handleLimits() const;

		private:
			bool m_AutoSave = false;
			std::string m_AutoSaveFile;
		};
	}
}