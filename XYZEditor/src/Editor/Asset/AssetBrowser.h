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

			virtual void OnImGuiRender(bool& open) override;

			void SetBaseDirectory(const std::string& path);
		
			Ref<Asset> GetSelectedAsset() const;

		private:
			void     registerFileTypeExtensions();
			void     createAsset() const;
			void     rightClickMenu() const;
			void     dragAndDrop(const std::filesystem::path& path) const;

			uint32_t extensionToTexCoordsIndex(const std::string& extension) const;
	

			void	 renderTopPanel();
			void	 processCurrentDirectory();
			void	 processDirectoryTree(const DirectoryNode& parentNode);

		private:
			std::filesystem::path m_SelectedFile;
			std::filesystem::path m_RightClickedFile;
			std::filesystem::path m_BaseDirectory;
			
			DirectoryTree		  m_DirectoryTree;

			std::unordered_map<std::string, uint32_t> m_FileTypeExtensions;
		private:
			// Style
			enum FileType
			{
				Animation = 4,
				Scene,
				Jpg,
				Png,
				Arrow,
				Mesh,
				Texture,
				SubTexture,
				Folder,
				Shader,
				Material,
				Script,
				NumTypes
			};
			
			glm::vec2 m_IconSize;
			glm::vec2 m_ArrowSize;

			float m_SplitterWidth;
		};
	}
}