#pragma once
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Editor/EditorPanel.h"

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

			size_t      extensionToTexCoordsIndex(const std::string& extension) const;
			std::string getUniqueAssetName(const char* fileName, const char* extension) const;
		

			void		renderTopPanel();
			void		processCurrentDirectory();
			void		processDirectoryTree(const std::filesystem::path& dirPath) const;
		private:	
			bool m_ViewportHovered;
			bool m_ViewportFocused;

			Ref<XYZ::Texture2D> m_Texture;

			std::filesystem::path			  m_SelectedFile;
			std::filesystem::path			  m_RightClickedFile;
			std::filesystem::path			  m_BaseDirectory;
			std::filesystem::path			  m_CurrentDirectory;
			std::deque<std::filesystem::path> m_DirectoriesVisited;


			std::unordered_map<std::string, size_t> m_FileTypeExtensions;



		private:
			// Style
			enum FileType
			{
				LeftArrow,
				RightArrow,
				Folder,
				Scene,
				Texture,
				SubTexture,
				Material,
				Shader,
				Font,
				Audio,
				Script,
				Mesh,
				Animation,
				Png,
				Jpg,
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
			struct UV
			{
				glm::vec2 UV0, UV1;

				static UV Calculate(const glm::vec2& coords, const glm::vec2& size, const glm::vec2& textureSize);
			};
			glm::vec2 m_IconSize;
			glm::vec2 m_ArrowSize;
			UV		  m_TexCoords[NumTypes];
			glm::vec4 m_Colors[NumColors];
		};
	}
}