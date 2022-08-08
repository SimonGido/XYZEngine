#pragma once

#include "Editor/EditorPanel.h"

#include "EditorConsoleSink.h"

namespace XYZ {
	namespace Editor {
		
		class EditorConsolePanel : public EditorPanel
		{
		public:
			EditorConsolePanel(std::string name);

			virtual void OnImGuiRender(bool& open) override;
			virtual void OnUpdate(Timestep ts) override;
			virtual bool OnEvent(Event& e) override;

			virtual void SetSceneContext(const Ref<Scene>& scene) override;
		
			MessageStream GetStream() { return MessageStream(&m_Messages, 500); }

		private:
			std::deque<ConsoleMessage> m_Messages;
		};
	}
}