#pragma once

#include "XYZ/Renderer/Texture.h"
#include "XYZ/Renderer/SpriteSheet.h"

#include <filesystem>

namespace XYZ {
	namespace Editor {
		class DirectoryNode
		{
		public:
			DirectoryNode(std::filesystem::path path, const UV& texCoords, const Ref<Texture2D>& texture);
			
			void SetPath(const std::filesystem::path& path);
			bool OnImGuiRender(glm::vec2 size) const;
			bool IsDirectory()				   const { return std::filesystem::is_directory(m_Path); }

			const std::filesystem::path& GetPath() const { return m_Path; }
			const std::string&			 GetName() const { return m_Name; }

			std::vector<DirectoryNode>::iterator	   begin()	     { return m_Nodes.begin(); }
			std::vector<DirectoryNode>::iterator	   end()		 { return m_Nodes.end(); }
			std::vector<DirectoryNode>::const_iterator begin() const { return m_Nodes.begin(); }
			std::vector<DirectoryNode>::const_iterator end()   const { return m_Nodes.end(); }

		private:
			std::filesystem::path	   m_Path;
			UV						   m_TexCoords;
			Ref<Texture2D>			   m_Texture;
			std::string				   m_Name;
			std::vector<DirectoryNode> m_Nodes;

			friend class DirectoryTree;
		};


		class DirectoryTree
		{
		public:
			enum FileType
			{
				Animation = 4,
				Scene,
				Jpg,
				Png,
				Arrow,
				Mesh,
				Texture,
				SubTexture,
				Folder,
				Shader,
				Material,
				Script,
				NumTypes
			};


			DirectoryTree(const std::filesystem::path& path);

			void SetCurrentNode(const DirectoryNode& node);
			void SetCurrentNode(const std::filesystem::path& path);

			void Undo();
			void Redo();

			bool UndoEmpty() const { return m_UndoDirectories.empty(); }
			bool RedoEmpty() const { return m_RedoDirectories.empty(); }

			const DirectoryNode& GetRoot()		  const { return m_Root; }
			const DirectoryNode& GetCurrentNode() const { return *m_CurrentNode; }
		private:
			void				 processDirectory(std::vector<DirectoryNode>& nodes, const std::filesystem::path& dirPath);
			const DirectoryNode* findNode(const std::filesystem::path& path, const DirectoryNode& node) const;
		private:
			DirectoryNode		 m_Root;
			const DirectoryNode* m_CurrentNode;

			std::deque<const DirectoryNode*> m_UndoDirectories;
			std::deque<const DirectoryNode*> m_RedoDirectories;
		};
	}
}