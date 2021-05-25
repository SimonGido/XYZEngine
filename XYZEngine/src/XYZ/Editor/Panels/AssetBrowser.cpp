#include "stdafx.h"
#include "AssetBrowser.h"

#include "XYZ/InGui/InGui.h"

#include <filesystem>

namespace XYZ {
	namespace Editor {
		AssetBrowser::AssetBrowser()
		{
		}
		void AssetBrowser::OnUpdate()
		{
			if (InGui::Begin("Assets"))
			{
				processDirectory("Assets");
			}
			InGui::End();
		}
		void AssetBrowser::processDirectory(const std::string& path)
		{
			for (auto it : std::filesystem::directory_iterator(path))
			{
				if (it.is_directory())
				{
					
				}
			}
		}
	}
}