#pragma once
#include "XYZ/Renderer/SubTexture.h"

#include "XYZ/InGui/InGui.h"

namespace XYZ {
	namespace Editor {
		class AssetBrowser
		{
		public:
			AssetBrowser();

			void SetPath(const std::string& path) { m_Path = path; };
			void OnUpdate();

		private:
			void processDirectory(const std::string& path);
	
		private:
			enum Type
			{
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

			Ref<XYZ::SubTexture> m_SubTextures[NumTypes];
			Ref<XYZ::Texture> m_Texture;
			glm::vec2 m_IconSize;
			std::string m_Path;
			std::deque<std::string> m_DirectoriesVisited;


			InGuiWindow* m_Window;

			struct Config
			{
				Config();
				void Begin(size_t colorID, size_t highlightID);
				void End();

				enum Color
				{
					BackArrow,
					BackArrowHighlight,
					Folder,
					FolderHighlight,
					Shader,
					ShaderHighlight,
					Script,
					ScriptHighlight,
					Material,
					MaterialHighlight,
					NumColors
				};
				glm::vec4 Colors[NumColors];

			private:
				glm::vec4 OldColor;
				glm::vec4 OldHighlight;
				bool IsBegin;
			};

			Config m_Config;
		};
	}
}