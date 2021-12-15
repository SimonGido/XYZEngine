#pragma once
#include "Editor/EditorPanel.h"

namespace XYZ {
	namespace Editor {
		class SceneHierarchyPanel : public EditorPanel
		{
		public:
			SceneHierarchyPanel(std::string name);
			~SceneHierarchyPanel();
			
			virtual void OnImGuiRender(bool& open) override;
			virtual bool OnEvent(Event& e) override;

			virtual void SetSceneContext(const Ref<Scene>& scene) override;

		private:
			void drawEntityNode(const SceneEntity& entity);
			void dragAndDrop(const SceneEntity& entity);
		private:
			Ref<Scene>				    m_Context;
		};
	}
}