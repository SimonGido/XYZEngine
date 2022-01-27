#pragma once
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/SpriteSheet.h"

#include "Editor/EditorPanel.h"
#include "Editor/Asset/DirectoryTree.h"


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
		
			Ref<Asset> GetSelectedAsset() const;

		private:
			void     createAsset() const;
			void     rightClickMenu() const;
			void     dragAndDrop(const std::filesystem::path& path) const;


			void	 renderTopPanel();
			void	 processCurrentDirectory();
			void	 processDirectoryTree(const DirectoryNode& parentNode);


			void	 onFileChange(const std::wstring& filePath);
			void	 onFileAdded(const std::wstring& filePath);
			void	 onFileRemoved(const std::wstring& filePath);
			void	 onFileRenamed(const std::wstring& filePath);
		private:
			std::filesystem::path m_SelectedFile;
			std::filesystem::path m_RightClickedFile;
			std::filesystem::path m_BaseDirectory;
			
			DirectoryTree		  m_DirectoryTree;
			std::mutex			  m_DirectoryTreeMutex;
		private:		
			glm::vec2 m_IconSize;
			glm::vec2 m_ArrowSize;

			float m_SplitterWidth;
		};
	}
}