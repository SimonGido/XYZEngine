#pragma once

#include "XYZ/Renderer/Texture.h"
#include "XYZ/Renderer/SpriteSheet.h"

#include <glm/glm.hpp>

#include <filesystem>

namespace XYZ {
	namespace Editor {

		class ImGuiFile
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
			ImGuiFile(std::filesystem::path path, const UV& texCoords, const Ref<Texture2D>& texture);

			State Render(const char* dragName, glm::vec2 size);
	
			void AddFile(const ImGuiFile& file);
			void EmplaceFile(std::filesystem::path path, const UV& texCoords, const Ref<Texture2D>& texture);
			void RemoveFile(const std::filesystem::path& path);
			void RemoveFile(std::vector<ImGuiFile>::const_iterator it);

			void Rename(std::filesystem::path path);
			void Delete();
			
			bool IsDirectory()		 const { return std::filesystem::is_directory(m_Path); }
			bool HasSubdirectories() const { return m_SubdirectoryCount != 0; }
			bool Empty()			 const { return m_Files.empty(); }


			const std::string&			  GetName()		 const { return m_Name; }
			const std::string&			  GetExtension() const { return m_Extension; }
			const std::filesystem::path&  GetPath()		 const { return m_Path; }
			const std::vector<ImGuiFile>& GetFiles()	 const { return m_Files; }

			std::vector<ImGuiFile>::iterator	   begin()		 { return m_Files.begin(); }
			std::vector<ImGuiFile>::iterator	   end()		 { return m_Files.end(); }
			std::vector<ImGuiFile>::const_iterator begin() const { return m_Files.begin(); }
			std::vector<ImGuiFile>::const_iterator end()   const { return m_Files.end(); }

		private:
			void finishEditing();

		private:
			std::filesystem::path  m_Path;
			Ref<Texture2D>		   m_Texture;
			UV					   m_TexCoords;
								   
			std::string			   m_PathStr;
			std::string			   m_Name;
			std::string			   m_Extension;

			std::vector<ImGuiFile> m_Files;

			bool m_EditingName			 = false;
			bool m_FocusedEdit			 = false;
			uint16_t m_SubdirectoryCount = 0;

			char m_NameBuffer[_MAX_FNAME];
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

		class ImGuiFileManager
		{
		public:
			ImGuiFileManager(const std::filesystem::path& path);
			ImGuiFileManager(const ImGuiFileManager& other) = delete;
			ImGuiFileManager(ImGuiFileManager&& other)  noexcept;


			ImGuiFileManager& operator=(const ImGuiFileManager& other) = delete;
			ImGuiFileManager& operator=(ImGuiFileManager&& other)  noexcept;

			void AddFile(const std::filesystem::path& path);
			void RemoveFile(const std::filesystem::path& path);
			void SetCurrentFile(const std::filesystem::path& path);
			
			void RenderCurrentDirectory(const char* dragName, glm::vec2 iconSize);
			void RenderDirectoryTree();

			void Undo();
			void Redo();

			bool IsUndoEmpty() const { return m_UndoDirectories.empty(); }
			bool IsRedoEmpty() const { return m_RedoDirectories.empty(); }

			void RegisterExtension(const std::string& ext, const FileExtensionInfo& extInfo);

			const ImGuiFile* GetLeftClickedFile()		const { return m_LeftClickedFile;}
			const ImGuiFile* GetLeftDoubleClickedFile() const { return m_LeftDoubleClickedFile;}
			const ImGuiFile* GetRightClickedFile()		const { return m_RightClickedFile;}

			const ImGuiFile& GetRoot()		  const { return m_Root; }
			const ImGuiFile& GetCurrentFile() const { return *m_CurrentFile; }
		private:
			void					  renderDirectoryTree(ImGuiFile& file, const FileExtensionInfo& extInfo);
			
			void					  setCurrentFile(ImGuiFile& file);
			void					  processDirectory(ImGuiFile& file, const std::filesystem::path& dirPath);
			ImGuiFile*				  findFile(const std::filesystem::path& path, ImGuiFile& file) const;
			const FileExtensionInfo*  findExtensionInfo(const std::filesystem::path& path) const;
		
		private:
			ImGuiFile  m_Root;
			ImGuiFile* m_CurrentFile;

			std::deque<ImGuiFile*> m_UndoDirectories;
			std::deque<ImGuiFile*> m_RedoDirectories;


			ImGuiFile* m_LeftDoubleClickedFile = nullptr;
			ImGuiFile* m_LeftClickedFile	   = nullptr;
			ImGuiFile* m_RightClickedFile	   = nullptr;


			std::unordered_map<std::string, FileExtensionInfo> m_Extensions;

			bool m_ExtensionsUpdated = false;
		};
	}
}