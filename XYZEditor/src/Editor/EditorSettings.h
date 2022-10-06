#pragma once


#include <glm/glm.hpp>


namespace XYZ {
	namespace Editor {
		
		// TODO:
		class EditorSettings
		{
		public:
			bool Deserialize(const std::filesystem::path& path) { return false; };
			void Serialize(const std::filesystem::path& path) {};


			glm::vec2 AssetBrowserIconSize;
			glm::vec2 AssetBrowserArrowSize;
		};
	}
}