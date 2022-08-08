#pragma once

#include "XYZ/Renderer/Texture.h"
#include "XYZ/Renderer/SpriteSheet.h"

#include <glm/glm.hpp>

#include <filesystem>

namespace XYZ {
	namespace Editor {

		class File
		{
		public:
			enum class State
			{
				None,
				LeftClicked,
				RightClicked,
				LeftDoubleClicked
			};

		public:
			File(std::filesystem::path path, const UV& texCoords, const Ref<Texture2D>& texture);

			State OnImGuiRender(const char* dragName, glm::vec2 size);
	
			void Rename(std::filesystem::path path);
			void Delete();
			bool IsDirectory() const { return std::filesystem::is_directory(m_Path); }
			bool Empty()	   const { return m_Files.empty(); }


			const std::string&			 GetName() const { return m_Name; }
			const std::filesystem::path& GetPath() const { return m_Path; }

			std::vector<File>::iterator		  begin()		{ return m_Files.begin(); }
			std::vector<File>::iterator		  end()		    { return m_Files.end(); }
			std::vector<File>::const_iterator begin() const { return m_Files.begin(); }
			std::vector<File>::const_iterator end()   const { return m_Files.end(); }

		private:
			void finishEditing();

		private:
			std::filesystem::path m_Path;
			Ref<Texture2D>		  m_Texture;
			UV					  m_TexCoords;

			std::string			  m_PathStr;
			std::string			  m_Name;
			std::string			  m_Extension;

			std::vector<File>     m_Files;

			bool m_EditingName = false;
			bool m_FocusedEdit = false;

			char m_NameBuffer[_MAX_FNAME];

			friend class FileManager;
		};

		struct FileExtensionInfo
		{
			using FileActionFn = std::function<bool(const std::filesystem::path&)>;

			Ref<Texture2D> Texture;
			UV			   TexCoords;

			FileActionFn   HoverAction;
			FileActionFn   LeftClickAction;
			FileActionFn   LeftDoubleClickAction;
			FileActionFn   RightClickAction;
		};

		class FileManager
		{
		public:
			FileManager(const std::filesystem::path& path);
			FileManager(const FileManager& other) = delete;
			FileManager(FileManager&& other)  noexcept;


			FileManager& operator=(const FileManager& other) = delete;
			FileManager& operator=(FileManager&& other)  noexcept;

			void AddFile(const std::filesystem::path& path);
			void RemoveFile(const std::filesystem::path& path);
			void SetCurrentFile(const std::filesystem::path& path);

			void ImGuiRenderCurrentDir(const char* dragName, glm::vec2 iconSize);
			void ImGuiRenderDirTree();

			void Undo();
			void Redo();

			bool IsUndoEmpty() const { return m_UndoDirectories.empty(); }
			bool IsRedoEmpty() const { return m_RedoDirectories.empty(); }

			void RegisterExtension(const std::string& ext, const FileExtensionInfo& extInfo);

			const File& GetRoot() const { return m_Root; }
		private:
			void					  imGuiRenderDirTree(File& file, const FileExtensionInfo& extInfo);
			
			void					  setCurrentFile(File& file);
			void					  processDirectory(std::vector<File>& files, const std::filesystem::path& dirPath);
			File*					  findNode(const std::filesystem::path& path, File& file) const;
			const FileExtensionInfo*  findExtensionInfo(const std::filesystem::path& path) const;
		
		private:
			File  m_Root;
			File* m_CurrentFile;

			std::deque<File*> m_UndoDirectories;
			std::deque<File*> m_RedoDirectories;


			File* m_LeftDoubleClickedFile = nullptr;
			File* m_LeftClickedFile = nullptr;
			File* m_RightClickedFile = nullptr;


			std::unordered_map<std::string, FileExtensionInfo> m_Extensions;
		};
	}
}