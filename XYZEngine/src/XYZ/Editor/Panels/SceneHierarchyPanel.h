#pragma once

#include "XYZ/Scene/Scene.h"

namespace XYZ {
	namespace Editor {
		class SceneHierarchyPanel
		{
		public:
			SceneHierarchyPanel();
			~SceneHierarchyPanel();

			void OnUpdate();
			void OnEvent(Event& event);

			void SetContext(const Ref<Scene>& context);
		private:
			bool onMouseButtonPress(MouseButtonPressEvent& event);
			bool onKeyPressed(KeyPressedEvent& event);

		private:
			Ref<Scene> m_Context;
			bool* m_Open;
		};
	}
}