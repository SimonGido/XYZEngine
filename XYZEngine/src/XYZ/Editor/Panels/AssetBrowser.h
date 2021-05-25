#pragma once
#include "XYZ/Renderer/SubTexture.h"


namespace XYZ {
	namespace Editor {
		class AssetBrowser
		{
		public:
			AssetBrowser();

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
		};
	}
}