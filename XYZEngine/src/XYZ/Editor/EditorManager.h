#pragma once
#include "EditorPanel.h"

namespace XYZ {
	namespace Editor {
		class EditorManager
		{
		public:
			void OnImGuiRender();
			void OnUpdate(Timestep ts);
			bool OnEvent(Event& e);

			void SetSceneContext(const Ref<Scene>& scene);

		
			template <typename T>
			void RegisterPanel(std::string name, bool open = true);

			template <typename T>
			Ref<T> GetPanel(const std::string& name) const;

		private:
			Ref<EditorPanel> getPanel(const std::string& name) const;

		private:
			struct View
			{
				Ref<EditorPanel> Panel;
				bool			 Open;
			};
			std::vector<View> m_EditorPanels;
			Ref<Scene>		  m_Scene;
		};
		template<typename T>
		inline void EditorManager::RegisterPanel(std::string name, bool open)
		{
			static_assert(std::is_base_of_v<EditorPanel, T>, "Type T must inherit from EditorPanel");
			XYZ_ASSERT(getPanel(name).Raw() == nullptr, "Panel with the same name already exists");
			Ref<T> panel = Ref<T>::Create(std::move(name));
			if (m_Scene.Raw())
				panel->SetSceneContext(m_Scene);
			m_EditorPanels.push_back({ panel, open });
		}

		template<typename T>
		inline Ref<T> EditorManager::GetPanel(const std::string& name) const
		{
			auto panel = getPanel(name);
			XYZ_ASSERT(panel.Raw() != nullptr, "Panel does not exist");
			return panel.As<T>();
		}
	}
}