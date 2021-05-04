#pragma once

#include "XYZ/Scene/Scene.h"
#include "XYZ/Editor/EditorUI.h"

namespace XYZ {
	namespace Editor {
		class SceneHierarchyPanel : public EditorUI
		{
		public:
			SceneHierarchyPanel(const std::string& filepath);
			~SceneHierarchyPanel();

			virtual void OnUpdate(Timestep ts) override;
			virtual void OnReload() override;
			virtual void SetupUI() override;

			void OnEvent(Event& event);

			void SetContext(Ref<Scene> context);
		private:
			void rebuildTree();
			void updateTree();

			bool onMouseButtonPress(MouseButtonPressEvent& event);
			bool onKeyPressed(KeyPressedEvent& event);

		private:
			Ref<Scene> m_Context;
			bUIWindow* m_Window;
			bUITree* m_Tree;
			bUIImage* m_Image;
		};
	}
}