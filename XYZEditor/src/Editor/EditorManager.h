#pragma once
#include "EditorPanel.h"

#include "XYZ/ImGui/ImGuiDialog.h"

namespace XYZ {
	namespace Editor {
		class EditorManager
		{
		public:
			void OnImGuiRender();
			void OnUpdate(Timestep ts);
			bool OnEvent(Event& e);
			void Clear();

			void SetSceneContext(const Ref<Scene>& scene);
		
			template <typename T>
			std::shared_ptr<T> RegisterPanel(std::string name, bool open = true);

			template <typename T>
			std::shared_ptr<T> GetPanel(const std::string& name) const;

			void			   SetOpen(size_t index, bool open) { m_EditorPanels[index].Open = open; }
			bool			   IsOpen(size_t index)		  const { return m_EditorPanels[index].Open; }
			size_t			   GetNumEditorPanels()		  const { return m_EditorPanels.size(); }
			const std::string& GetPanelName(size_t index) const { return m_EditorPanels[index].Panel->GetName(); }
		private:
			std::shared_ptr<EditorPanel> getPanel(const std::string& name) const;

			void displayMenuBar();

		private:
			struct View
			{
				std::shared_ptr<EditorPanel> Panel;
				bool						 Open;
			};
			std::vector<View>		 m_EditorPanels;
			Ref<Scene>				 m_Scene;
			ImGuiDialogQueue		 m_DialogQueue;
		};
		template<typename T>
		inline std::shared_ptr<T> EditorManager::RegisterPanel(std::string name, bool open)
		{
			static_assert(std::is_base_of_v<EditorPanel, T>, "Type T must inherit from EditorPanel");
			XYZ_ASSERT(getPanel(name) == nullptr, "Panel with the same name already exists");
			std::shared_ptr<T> panel = std::make_shared<T>(std::move(name));
			if (m_Scene.Raw())
				panel->SetSceneContext(m_Scene);

			m_EditorPanels.push_back({ panel, open });
			return panel;
		}

		template<typename T>
		inline std::shared_ptr<T> EditorManager::GetPanel(const std::string& name) const
		{
			auto panel = getPanel(name);
			XYZ_ASSERT(panel != nullptr, "Panel does not exist");
			return std::static_pointer_cast<T>(panel);
		}
	}
}