#pragma once
#include "XYZ/Renderer/SubTexture.h"

#include <deque>

namespace XYZ {
	namespace Editor {
		class AssetBrowser
		{
		public:
			AssetBrowser();

			void SetPath(const std::string& path);
			void OnImGuiRender();

		private:
			void processDirectory(const std::string& path);
	
		private:
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
				SkeletalMesh,
				NumTypes
			};

			glm::vec4 m_TexCoords[NumTypes];
			Ref<XYZ::Texture> m_Texture;

			glm::vec2 m_IconSize;
			glm::vec2 m_ArrowSize;
			char m_Path[_MAX_PATH];
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