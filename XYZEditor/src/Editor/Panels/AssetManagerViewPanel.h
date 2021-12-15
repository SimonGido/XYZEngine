#pragma once
#include "Editor/EditorPanel.h"

namespace XYZ {
	namespace Editor {
		class AssetManagerViewPanel : public EditorPanel
		{
		public:
			AssetManagerViewPanel(std::string name);
			~AssetManagerViewPanel();

			virtual void OnImGuiRender(bool& open) override;

		private:
			static void displayAllMetadata();
			static void displaySearchedMetadata(const std::string& searchString);
		};
	}
}