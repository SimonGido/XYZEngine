#pragma once

#include "XYZ/Scene/Scene.h"

namespace XYZ {
	namespace Editor {
		class SceneHierarchyPanel
		{
		public:
			SceneHierarchyPanel();
			~SceneHierarchyPanel();

			void OnImGuiRender(bool& open);
			void SetContext(const Ref<Scene>& context);
		
		private:
			void drawEntityNode(const SceneEntity& entity);
			void dragAndDrop(const SceneEntity& entity);
		private:
			Ref<Scene>				    m_Context;
		};
	}
}