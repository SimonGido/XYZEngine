#pragma once
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/SpriteSheet.h"

#include "Editor/EditorPanel.h"

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
			void		registerFileTypeExtensions();
			void		createAsset() const;
			void		rightClickMenu() const;
			void		dragAndDrop(const std::filesystem::path& path) const;

			uint32_t    extensionToTexCoordsIndex(const std::string& extension) const;
			std::string createUniqueAssetName(const char* fileName, const char* extension) const;
		

			void		renderTopPanel();
			void		processCurrentDirectory();
			void		processDirectoryTree(const std::filesystem::path& dirPath) const;
		private:	
			bool m_ViewportHovered;
			bool m_ViewportFocused;

			std::filesystem::path			  m_SelectedFile;
			std::filesystem::path			  m_RightClickedFile;
			std::filesystem::path			  m_BaseDirectory;
			std::filesystem::path			  m_CurrentDirectory;
			std::deque<std::filesystem::path> m_DirectoriesVisited;


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

			enum Color
			{
				ArrowColor,
				IconColor,
				DisabledColor,
				HoverColor,
				ClickColor,
				NumColors
			};
			
			glm::vec2 m_IconSize;
			glm::vec2 m_ArrowSize;

			
			glm::vec4 m_Colors[NumColors];

			float m_Widths[2] = { 300.0f, 400.0f };
		};
	}
}