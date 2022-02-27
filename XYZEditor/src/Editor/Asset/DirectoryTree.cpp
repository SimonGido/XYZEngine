#include "stdafx.h"
#include "DirectoryTree.h"

#include "EditorLayer.h"

namespace XYZ {
	namespace Editor {

		
		static std::unordered_map<std::string, ED::Sprites> s_FileTypeExtensions = {
				{"xyz",		ED::SceneIcon},
				{"tex",		ED::TextureIcon},
				{"subtex",	ED::MeshIcon},
				{"mat",		ED::MaterialIcon},
				{"shader",	ED::ShaderIcon},
				{"cs",		ED::ScriptIcon},
				{"anim",	ED::AnimationIcon},
				{"png",		ED::PngIcon},
				{"jpg",		ED::JpgIcon},
				{"mesh",	ED::MeshIcon}
		};

		static ED::Sprites ExtensionToFileType(const std::string& extension)
		{
			auto it = s_FileTypeExtensions.find(extension);
			if (it != s_FileTypeExtensions.end())
				return it->second;
			return ED::NumIcons;
		}

		DirectoryNode::DirectoryNode(std::filesystem::path path, const UV& texCoords, const Ref<Texture2D>& texture, const uint32_t depth)
			:
			m_Path(std::move(path)),
			m_TexCoords(texCoords),
			m_Texture(texture),
			m_Depth(depth)
		{
			m_Name = m_Path.filename().string();
		}


		void DirectoryNode::SetPath(const std::filesystem::path& path)
		{
			if (m_Path == path)
				return;

			FileSystem::Rename(m_Path.string(), Utils::GetFilenameWithoutExtension(m_Name));
			m_Path = path;
			m_Name = m_Path.filename().string();
			if (IsDirectory())
			{
				for (auto& node : m_Nodes)
				{
					node.m_Path = m_Path / node.m_Name;
					node.SetPath(node.m_Path);
				}
			}
		}

		std::string DirectoryNode::GetPathString() const
		{
			std::string result = m_Path.string();
			std::replace(result.begin(), result.end(), '\\', '/');
			return result;
		}

		void DirectoryNode::OnImGuiRender(const char* dragName, glm::vec2 size, bool& leftClick, bool& rightClick, bool& leftDoubleClick)
		{
			if (!m_Texture.Raw())
				return;

			const auto& preferences = EditorLayer::GetData();
			leftClick = UI::ImageButtonTransparent(m_Name.c_str(), m_Texture->GetImage(), size,
				preferences.Color[ED::IconHoverColor], preferences.Color[ED::IconClickColor], preferences.Color[ED::IconColor],
				m_TexCoords[0], m_TexCoords[1]);

			leftDoubleClick = UI::Utils::IsItemDoubleClicked(ImGuiMouseButton_Left);

			std::string path = m_Path.string();
			UI::DragDropSource(dragName, path.c_str(), path.size() + 1);

			rightClick = ImGui::IsItemClicked(ImGuiMouseButton_Right);

			if (m_EditingName && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				m_Name = m_NameBuffer;
				auto newPath = m_Path.parent_path() / m_Name;
				SetPath(newPath);
				m_EditingName = false;
				m_FocusedEdit = false;
			}

			UI::ScopedID id(m_Name.c_str());
			if (!m_EditingName)
			{
				ImGui::TextWrapped(m_Name.c_str());
				m_EditingName = UI::Utils::IsItemDoubleClicked(ImGuiMouseButton_Left);
				if (m_EditingName)
				{
					memcpy(m_NameBuffer, m_Name.c_str(), m_Name.size());
					m_NameBuffer[m_Name.size()] = '\0';
				}
			}
			else
			{
				if (!m_FocusedEdit)
				{
					ImGui::SetKeyboardFocusHere(0); // Set Focus on Input Text
					m_FocusedEdit = true;
				}
				ImGui::InputText("##InputName", m_NameBuffer, _MAX_FNAME);
			}
			
		}
		DirectoryNode* DirectoryTree::findNode(const std::filesystem::path& path, DirectoryNode& node) const
		{
			std::stack<DirectoryNode*> nodes;
			nodes.push(&node);
			while (!nodes.empty())
			{
				DirectoryNode* tmp = nodes.top();
				nodes.pop();
				if (tmp->GetPath() == path)
					return tmp;

				for (auto& child : *tmp)
					nodes.push(&child);
			}
			return nullptr;
		}

		
		DirectoryTree::DirectoryTree(const std::filesystem::path& path)
			:
			m_Root(path, {}, nullptr, 0)
		{
			m_CurrentNode = &m_Root;
			processDirectory(m_Root.m_Nodes, path, 1);
		}
		void DirectoryTree::Rebuild(const std::filesystem::path& path)
		{
			if (m_CurrentNode)
			{
				std::filesystem::path currentPath = m_CurrentNode->m_Path;
				m_Root = DirectoryNode(path, {}, nullptr, 0);
				processDirectory(m_Root.m_Nodes, path, 1);

				if (auto currentNode = findNode(currentPath, m_Root))
					m_CurrentNode = currentNode;
				else
					m_CurrentNode = &m_Root;
			}
			else
			{
				m_Root = DirectoryNode(path, {}, nullptr, 0);
				m_CurrentNode = &m_Root;
				processDirectory(m_Root.m_Nodes, path, 1);
			}
				
			m_UndoDirectories.clear();
			m_RedoDirectories.clear();
		}

		void DirectoryTree::SetCurrentNode(DirectoryNode& node)
		{
			m_UndoDirectories.push_back(m_CurrentNode);
			m_RedoDirectories.clear();
			m_CurrentNode = &node;
		}
		void DirectoryTree::SetCurrentNode(const std::filesystem::path& path)
		{
			DirectoryNode* found = findNode(path, m_Root);
			if (found)
			{
				m_UndoDirectories.push_back(m_CurrentNode);
				m_RedoDirectories.clear();
				m_CurrentNode = found;
			}
		}

		void DirectoryTree::AddNode(DirectoryNode& node, const std::filesystem::path& path)
		{
			std::filesystem::path currentNodePath = m_CurrentNode->m_Path;
			if (std::filesystem::is_directory(path))
			{
				const UV& folderTexCoords = EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::FolderIcon);
				node.m_Nodes.emplace_back(path, folderTexCoords, EditorLayer::GetData().IconsTexture, node.m_Depth + 1);
			}
			else
			{
				processFile(path, node.m_Nodes, node.m_Depth + 1);
			}
			m_CurrentNode = findNode(currentNodePath, m_Root);
		}

		template <typename TContainer, typename T>
		static void eraseAll(TContainer& container, const T& val)
		{
			for (auto it = container.begin(); it != container.end();)
			{
				if (*it == &val)
					it = container.erase(it);
				else
					it++;
			}
		}

		void DirectoryTree::RemoveNode(DirectoryNode& node, const std::filesystem::path& path)
		{
			std::filesystem::path currentNodePath = m_CurrentNode->m_Path;
			for (size_t i = 0; i < node.m_Nodes.size(); ++i)
			{
				auto& nodes = node.m_Nodes;
				if (nodes[i].m_Path == path)
				{			
					eraseAll(m_UndoDirectories, nodes[i]);
					eraseAll(m_RedoDirectories, nodes[i]);
					nodes.erase(nodes.begin() + i);
					return;
				}
			}
			m_CurrentNode = findNode(currentNodePath, m_Root);
		}

		void DirectoryTree::Undo()
		{
			m_RedoDirectories.push_back(m_CurrentNode);
			m_CurrentNode = m_UndoDirectories.back();
			m_UndoDirectories.pop_back();
		}
		void DirectoryTree::Redo()
		{
			m_UndoDirectories.push_back(m_CurrentNode);
			m_CurrentNode = m_RedoDirectories.back();
			m_RedoDirectories.pop_back();
		}

		DirectoryNode* DirectoryTree::FindNode(const std::filesystem::path& path)
		{
			return findNode(path, m_Root);
		}

		void DirectoryTree::processDirectory(std::vector<DirectoryNode>& nodes, const std::filesystem::path& dirPath, const uint32_t depth)
		{
			for (auto& it : std::filesystem::directory_iterator(dirPath))
			{
				if (it.is_directory())
				{
					const UV& folderTexCoords = EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::FolderIcon);
					nodes.emplace_back(it.path(), folderTexCoords, EditorLayer::GetData().IconsTexture, depth);		
					processDirectory(nodes.back().m_Nodes, it.path(), depth + 1);
				}
				else
				{
					processFile(it.path(), nodes, depth);
				}
			}
		}
		void DirectoryTree::processFile(const std::filesystem::path& path, std::vector<DirectoryNode>& nodes, const uint32_t depth)
		{
			ED::Sprites type = ExtensionToFileType(Utils::GetExtension(path.filename().string()));
			if (type == ED::TextureIcon)
			{
				const UV textureCoords = { glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 0.0f) };
				Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(path);
				nodes.emplace_back(path, textureCoords, texture, depth);
			}
			else if (type != ED::NumIcons) // Unknown file
			{
				const UV& fileTexCoords = EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(type);
				nodes.emplace_back(path, fileTexCoords, EditorLayer::GetData().IconsTexture, depth);
			}
		}
	}
}