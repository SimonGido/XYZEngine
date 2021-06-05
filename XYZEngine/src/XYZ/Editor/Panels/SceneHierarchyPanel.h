#pragma once

#include "XYZ/Scene/Scene.h"

namespace XYZ {
	namespace Editor {
		class SceneHierarchyPanel
		{
			using SceneEntitySelectedCallback = std::function<void(SceneEntity)>;
		public:
			SceneHierarchyPanel();
			~SceneHierarchyPanel();

			void OnImGuiRender();
			void SetContext(const Ref<Scene>& context);
			void SetEntitySelectedCallback(const SceneEntitySelectedCallback& callback) { m_Callback = callback; }

		private:
			void drawEntityNode(SceneEntity entity);

		private:
			SceneEntitySelectedCallback m_Callback;
			Ref<Scene> m_Context;
		};
	}
}