#pragma once
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/SpriteSheet.h"

#include "Editor/EditorPanel.h"

#include "Editor/Asset/ImGuiFile.h"

#include "XYZ/FileWatcher/FileWatcher.h"

#include <deque>
#include <filesystem>

namespace XYZ {
	namespace Editor {

		class AssetBrowser : public EditorPanel
		{
		public:
			AssetBrowser(std::string name);
			~AssetBrowser() override;

			virtual void OnImGuiRender(bool& open) override;
		
			void SetBaseDirectory(const std::string& path);
		
			Ref<Asset> GetSelectedAsset();

		private:
			bool rightClickMenu();

			void renderTopPanel();

			void onFileChange(FileWatcher::ChangeType type, const std::filesystem::path& filePath);

			void assetSelected(const std::filesystem::path& path);
			bool assetRightClickMenuFBX(const std::filesystem::path& path);
			bool assetRightClickMenuMESHSRC(const std::filesystem::path& path);
			bool assetRightClickMenuMESH(const std::filesystem::path& path);
			bool assetRightClickMenuANIMMESH(const std::filesystem::path& path);

			template <typename T, typename ...Args>
			static Ref<T> assetRightClickMenu(const std::filesystem::path& path, const std::string& assetName, const char* menuName, Args&& ...args);
		private:
			std::filesystem::path m_BaseDirectory;
			ImGuiFileManager	  m_FileManager;
		private:		
			glm::vec2  m_IconSize;
			glm::vec2  m_ArrowSize;
			Ref<Asset> m_SelectedAsset;

			float m_SplitterWidth;
		};
		template<typename T, typename ...Args>
		inline Ref<T> AssetBrowser::assetRightClickMenu(const std::filesystem::path& path, const std::string& assetName, const char* menuName, Args && ...args)
		{
			if (ImGui::MenuItem(menuName))
			{
				const std::string parentDir = path.parent_path().string();
				const std::string ext = "." + Utils::GetExtension(assetName);
				const std::string name = Utils::GetFilenameWithoutExtension(assetName);

				const std::string fullpath = FileSystem::UniqueFilePath(parentDir, name, ext.c_str());
				Ref<T> asset = AssetManager::CreateAsset<T>(Utils::GetFilename(fullpath), parentDir, std::forward<Args>(args)...);
				AssetManager::Serialize(asset->GetHandle());
				return asset;
			}
			return Ref<T>();
		}
	}
}