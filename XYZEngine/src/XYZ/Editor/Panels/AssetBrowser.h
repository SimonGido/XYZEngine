#pragma once
#include "XYZ/Renderer/SubTexture.h"

#include <deque>

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
			void processDirectory(const std::string& path);
			size_t assetTypeToTexCoordsIndex(AssetType type) const;

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
				NumTypes
			};

			glm::vec4 m_TexCoords[NumTypes];
			Ref<XYZ::Texture> m_Texture;

			glm::vec2 m_IconSize;
			glm::vec2 m_ArrowSize;
			char m_Path[_MAX_PATH];
			std::string m_SelectedFile;
			size_t m_PathLength;

			std::deque<std::string> m_DirectoriesVisited;


			enum Color
			{
				ArrowColor,
				ArrowInvalidColor,
				FolderColor,
				NumColors
			};
			glm::vec4 m_Colors[NumColors];
		};
	}
}