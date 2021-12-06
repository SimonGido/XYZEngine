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

			void SetPath(const std::string& path);
		
			Ref<Asset> GetSelectedAsset() const;

		private:
			void		registerFileTypeExtensions();
			void		createAsset() const;
			void		processDirectory();
			void		rightClickMenu() const;
			void		dragAndDrop(const std::filesystem::path& path) const;

			size_t      extensionToTexCoordsIndex(const std::string& extension) const;
			std::string getUniqueAssetName(const char* fileName, const char* extension) const;
		
		private:	
			bool m_ViewportHovered;
			bool m_ViewportFocused;

			Ref<XYZ::Texture2D> m_Texture;

			std::filesystem::path			  m_SelectedFile;
			std::filesystem::path			  m_RightClickedFile;
			std::filesystem::path			  m_CurrentDirectory;
			std::deque<std::filesystem::path> m_DirectoriesVisited;


			std::unordered_map<std::string, size_t> m_FileTypeExtensions;
		};
	}
}