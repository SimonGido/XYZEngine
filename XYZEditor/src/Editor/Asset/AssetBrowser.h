#pragma once
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/SpriteSheet.h"

#include "Editor/EditorPanel.h"

#include "Editor/Asset/FileManager.h"

#include "XYZ/FileWatcher/FileWatcher.h"

#include <deque>
#include <filesystem>

namespace XYZ {
	namespace Editor {

		class AssetBrowser : public EditorPanel
		{
		public:
			AssetBrowser(std::string name);
			~AssetBrowser() override;

			virtual void OnImGuiRender(bool& open) override;

			void SetBaseDirectory(const std::string& path);
		
			Ref<Asset> GetSelectedAsset();

		private:
			void createAsset();
			void rightClickMenu();
			bool tryClearClickedFiles();

			void renderTopPanel();

			void onFileChange(FileWatcher::ChangeType type, const std::filesystem::path& filePath);
		private:
			std::filesystem::path m_BaseDirectory;

			FileManager			  m_FileManager;
		private:		
			glm::vec2 m_IconSize;
			glm::vec2 m_ArrowSize;
			Ref<Asset> m_SelectedAsset;

			float m_SplitterWidth;
		};
	}
}