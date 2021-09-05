#pragma once
#include "XYZ/Renderer/SubTexture.h"

#include <deque>
#include <filesystem>

namespace XYZ {
	namespace Editor {
		class AssetBrowser
		{
			using AssetSelectedCallback = std::function<void(const Ref<Asset>&)>;
		public:
			AssetBrowser();

			void SetPath(const std::string& path);
			void OnImGuiRender();

			void SetAssetSelectedCallback(const AssetSelectedCallback& callback) { m_Callback = callback; }
			
			Ref<Asset> GetSelectedAsset() const;

		private:
			void		registerFileTypeExtensions();
			void		createAsset() const;
			void		processDirectory();
			void		rightClickMenu() const;
			void		dragAndDrop(const std::filesystem::path& path) const;

			size_t		assetTypeToTexCoordsIndex(AssetType type) const;
			size_t      extensionToTexCoordsIndex(const std::string& extension) const;
			std::string getUniqueAssetName(const char* fileName, const char* extension) const;
		
		private:
			AssetSelectedCallback m_Callback;
			
			bool m_ViewportHovered;
			bool m_ViewportFocused;

			enum Type
			{
				Arrow,
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

			glm::vec4		  m_TexCoords[NumTypes];
			Ref<XYZ::Texture> m_Texture;

			glm::vec2 m_IconSize;
			glm::vec2 m_ArrowSize;

			std::filesystem::path			  m_SelectedFile;
			std::filesystem::path			  m_RightClickedFile;
			std::filesystem::path			  m_CurrentDirectory;
			std::deque<std::filesystem::path> m_DirectoriesVisited;

			

			enum Color
			{
				ArrowColor,
				ArrowInvalidColor,
				FolderColor,
				NumColors
			};
			glm::vec4 m_Colors[NumColors];

			std::unordered_map<std::string, size_t> m_FileTypeExtensions;
		};
	}
}