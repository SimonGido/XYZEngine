#pragma once

#include "XYZ/Scene/Scene.h"

namespace XYZ {
	namespace Editor {
		class SceneHierarchyPanel
		{
		public:
			SceneHierarchyPanel();
			~SceneHierarchyPanel();

			void OnImGuiRender();
			void SetContext(const Ref<Scene>& context);

		private:
			void drawEntityNode(SceneEntity entity);

		private:
			Ref<Scene> m_Context;
		};
	}
}