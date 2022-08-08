#pragma once

#include "Editor/EditorPanel.h"


namespace XYZ {
	namespace Editor {
		class ScriptPanel : public EditorPanel
		{
		public:
			ScriptPanel(std::string name);

			virtual void OnImGuiRender(bool& open) override;

		private:

		};
	}
}