#pragma once

#include "XYZ/Renderer/Texture.h"
#include "XYZ/Renderer/SpriteSheet.h"

#include <filesystem>
#include <stack>

namespace XYZ {
	namespace Editor {
		class DirectoryNode
		{
		public:
			DirectoryNode(std::filesystem::path path, const UV& texCoords, const Ref<Texture2D>& texture, const uint32_t depth);
			
			void SetPath(const std::filesystem::path& path);
			void OnImGuiRender(const char* dragName, glm::vec2 size, bool& leftClick, bool& rightClick, bool& leftDoubleClick);


			const std::filesystem::path& GetPath()     const { return m_Path; }
			const std::string&			 GetName()     const { return m_Name; }
			const uint32_t				 GetDepth()    const { return m_Depth; }
			std::string					 GetPathString()  const;
			bool						 IsDirectory() const { return std::filesystem::is_directory(m_Path); }
			bool						 Empty()	   const { return m_Nodes.empty(); }
			size_t						 Size()		   const { return m_Nodes.size(); }

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
			uint32_t				   m_Depth;

			bool m_EditingName = false;
			bool m_FocusedEdit = false;

			char m_NameBuffer[_MAX_FNAME];

			friend class DirectoryTree;
		};


		class DirectoryTree
		{
		public:
			DirectoryTree(const std::filesystem::path& path);
			
			void Rebuild(const std::filesystem::path& path);

			void SetCurrentNode(DirectoryNode& node);
			void SetCurrentNode(const std::filesystem::path& path);
			void AddNode(DirectoryNode& node, const std::filesystem::path& path);
			void RemoveNode(DirectoryNode& node, const std::filesystem::path& path);

			void Undo();
			void Redo();

			bool UndoEmpty() const { return m_UndoDirectories.empty(); }
			bool RedoEmpty() const { return m_RedoDirectories.empty(); }

			DirectoryNode& GetRoot()		 { return m_Root; }
			DirectoryNode& GetCurrentNode()  { return *m_CurrentNode; }
			DirectoryNode* FindNode(const std::filesystem::path& path);
		private:
			void		   processDirectory(std::vector<DirectoryNode>& nodes, const std::filesystem::path& dirPath, const uint32_t depth);
			void		   processFile(const std::filesystem::path& path, std::vector<DirectoryNode>& nodes, const uint32_t depth);
			DirectoryNode* findNode(const std::filesystem::path& path, DirectoryNode& node) const;
		private:
			DirectoryNode  m_Root;
			DirectoryNode* m_CurrentNode;

			std::deque<DirectoryNode*> m_UndoDirectories;
			std::deque<DirectoryNode*> m_RedoDirectories;
		};
	
	}
}