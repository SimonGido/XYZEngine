#include "stdafx.h"
#include "DirectoryTree.h"

#include "EditorLayer.h"

namespace XYZ {
	namespace Editor {

		
		static std::unordered_map<std::string, ED::Sprites> s_FileTypeExtensions = {
				{"xyz",		ED::SceneIcon},
				{"tex",		ED::TextureIcon},
				{"subtex",	ED::TextureIcon},
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
			m_Path = path;
			m_Name = m_Path.filename().string();
		}
		bool DirectoryNode::OnImGuiRender(glm::vec2 size) const
		{
			if (!m_Texture.Raw())
				return false;

			const auto& preferences = EditorLayer::GetData();
			return UI::ImageButtonTransparent(m_Name.c_str(), m_Texture->GetImage(), size,
				preferences.Color[ED::IconHoverColor], preferences.Color[ED::IconClickColor], preferences.Color[ED::IconColor],
				m_TexCoords[0], m_TexCoords[1]);
		}
		const DirectoryNode* DirectoryTree::findNode(const std::filesystem::path& path, const DirectoryNode& node) const
		{
			std::stack<const DirectoryNode*> nodes;
			nodes.push(&node);
			while (!nodes.empty())
			{
				const DirectoryNode* tmp = nodes.top();
				nodes.pop();
				if (tmp->GetPath() == path)
					return tmp;

				for (const auto& child : *tmp)
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
			m_Root = DirectoryNode(path, {}, nullptr, 0);
			m_CurrentNode = &m_Root;
			processDirectory(m_Root.m_Nodes, path, 1);
			m_UndoDirectories.clear();
			m_RedoDirectories.clear();
		}

		void DirectoryTree::SetCurrentNode(const DirectoryNode& node)
		{
			m_UndoDirectories.push_back(m_CurrentNode);
			m_RedoDirectories.clear();
			m_CurrentNode = &node;
		}
		void DirectoryTree::SetCurrentNode(const std::filesystem::path& path)
		{
			const DirectoryNode* found = findNode(path, m_Root);
			if (found)
			{
				m_UndoDirectories.push_back(m_CurrentNode);
				m_RedoDirectories.clear();
				m_CurrentNode = found;
			}
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
					ED::Sprites type = ExtensionToFileType(Utils::GetExtension(it.path().filename().string()));
					if (type == ED::TextureIcon)
					{
						const UV textureCoords = { glm::vec2(0.0f), glm::vec2(1.0f) };
						Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(it.path());
						nodes.emplace_back(it.path(), textureCoords, texture, depth);
					}
					else if (type != ED::NumIcons) // Unknown file
					{
						const UV& fileTexCoords = EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(type);
						nodes.emplace_back(it.path(), fileTexCoords, EditorLayer::GetData().IconsTexture, depth);
					}
				}
			}
		}
		
	}
}