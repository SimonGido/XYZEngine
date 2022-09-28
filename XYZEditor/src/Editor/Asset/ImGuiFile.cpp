#include "stdafx.h"
#include "ImGuiFile.h"

#include "EditorLayer.h"

#include "XYZ/Utils/StringUtils.h"

namespace XYZ {
	namespace Editor {
		static constexpr const char* sc_DirectoryExtension = "dir";

		ImGuiFile::ImGuiFile(std::filesystem::path path, const UV& texCoords, const Ref<Texture2D>& texture)
			:
			m_Path(path),
			m_TexCoords(texCoords),
			m_Texture(texture)
		{
			m_PathStr = m_Path.string();
			m_Name = m_Path.filename().string();
			if (std::filesystem::is_directory(path))
				m_Extension = sc_DirectoryExtension;
			else
				m_Extension = Utils::GetExtension(m_PathStr);
			memset(m_NameBuffer, 0, _MAX_FNAME);
		}
		ImGuiFile::State ImGuiFile::Render(const char* dragName, glm::vec2 size)
		{
			if (!m_Texture.Raw())
				return State::None;

			State state = State::None;
			const auto& preferences = EditorLayer::GetData();
			if (UI::ImageButtonTransparent(
				m_Name.c_str(), 
				m_Texture->GetImage(), 
				size,
				preferences.Color[ED::IconHoverColor], 
				preferences.Color[ED::IconClickColor], 
				preferences.Color[ED::IconColor],
				m_TexCoords[0], 
				m_TexCoords[1]
			))
			{
				state = State::LeftClicked;
			}
			if (UI::Utils::IsItemDoubleClicked(ImGuiMouseButton_Left))
			{
				state = State::LeftDoubleClicked;
			}
			UI::DragDropSource(dragName, m_PathStr.c_str(), m_PathStr.size() + 1);
			
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
			{
				state = State::RightClicked;
			}

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && m_EditingName)
				finishEditing();
			

			UI::ScopedID id(m_Name.c_str());
			if (!m_EditingName)
			{
				ImGui::TextWrapped(m_Name.c_str());
				// Check if edit started
				m_EditingName = UI::Utils::IsItemDoubleClicked(ImGuiMouseButton_Left);
				if (m_EditingName)
				{
					memcpy(m_NameBuffer, m_Name.c_str(), m_Name.size());
					m_NameBuffer[m_Name.size()] = '\0';
				}
			}
			else
			{
				// Check if we are currently focusing on input
				if (!m_FocusedEdit)
				{
					ImGui::SetKeyboardFocusHere(0); // Set Focus on Input Text
					m_FocusedEdit = true;
				}
				ImGui::InputText("##InputName", m_NameBuffer, _MAX_FNAME);
			}

			return state;
		}
		void ImGuiFile::AddFile(const ImGuiFile& file)
		{
			m_Files.push_back(file);
			if (file.IsDirectory())
				m_SubdirectoryCount++;
		}
		void ImGuiFile::EmplaceFile(std::filesystem::path path, const UV& texCoords, const Ref<Texture2D>& texture)
		{
			m_Files.emplace_back(std::move(path), texCoords, texture);
			if (std::filesystem::is_directory(path))
				m_SubdirectoryCount++;
		}
		void ImGuiFile::RemoveFile(const std::filesystem::path& path)
		{
			for (size_t i = 0; i < m_Files.size(); ++i)
			{
				if (m_Files[i].m_Path == path)
				{
					if (m_Files[i].IsDirectory())
						m_SubdirectoryCount--;

					m_Files.erase(m_Files.begin() + i);
					return;
				}
			}
		}
		void ImGuiFile::RemoveFile(std::vector<ImGuiFile>::const_iterator it)
		{
			if (it->IsDirectory())
				m_SubdirectoryCount--;
			m_Files.erase(it);
		}
		void ImGuiFile::Rename(std::filesystem::path path)
		{
			if (m_Path == path)
				return;

			FileSystem::Rename(m_PathStr, Utils::GetFilenameWithoutExtension(m_Name));
			m_Path = std::move(path);
			m_PathStr = m_Path.string();
			m_Name = m_Path.filename().string();
			if (IsDirectory())
			{
				for (auto& file : m_Files)
				{
					auto newPath = m_Path / file.m_Name;
					file.Rename(newPath);
				}
			}
		}
		void ImGuiFile::Delete()
		{
			FileSystem::DeleteFileAtPath(m_PathStr);
			if (IsDirectory())
			{
				for (auto& file : m_Files)
				{
					file.Delete();
				}
			}
		}
		void ImGuiFile::finishEditing()
		{
			m_Name = m_NameBuffer;
			auto newPath = m_Path.parent_path() / m_Name;
			Rename(newPath);
			m_EditingName = false;
			m_FocusedEdit = false;
		}
		ImGuiFileManager::ImGuiFileManager(const std::filesystem::path& path)
			:
			m_Root(path, {}, nullptr)
		{
			m_CurrentFile = &m_Root;
			processDirectory(m_Root, path);
		}
		
		ImGuiFileManager::ImGuiFileManager(ImGuiFileManager&& other) noexcept
			:
			m_Root(std::move(other.m_Root)),
			m_UndoDirectories(std::move(other.m_UndoDirectories)),
			m_RedoDirectories(std::move(other.m_RedoDirectories))
		{
			m_CurrentFile = findFile(other.m_CurrentFile->GetPath(), m_Root);
		}
		
		ImGuiFileManager& ImGuiFileManager::operator=(ImGuiFileManager&& other) noexcept
		{
			m_Root = std::move(other.m_Root);
			m_UndoDirectories = std::move(other.m_UndoDirectories);
			m_RedoDirectories = std::move(other.m_RedoDirectories);
			m_CurrentFile = findFile(other.m_CurrentFile->GetPath(), m_Root);
			return *this;
		}
		void ImGuiFileManager::AddFile(const std::filesystem::path& path)
		{
			std::filesystem::path currentNodePath = m_CurrentFile->GetPath();
			auto ext = findExtensionInfo(path);
			if (ext)
			{
				auto file = findFile(path.parent_path(), m_Root);
				file->EmplaceFile(path, ext->TexCoords, ext->Texture);
			}
			m_CurrentFile = findFile(currentNodePath, m_Root);

			m_LeftClickedFile = nullptr;
			m_LeftDoubleClickedFile = nullptr;
			m_RightClickedFile = nullptr;
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

		void ImGuiFileManager::RemoveFile(const std::filesystem::path& path)
		{
			std::filesystem::path currentNodePath = m_CurrentFile->GetPath();
			auto parentFile = findFile(path.parent_path(), m_Root);

			auto& files = parentFile->GetFiles();
			for (auto it = files.begin(); it != files.end(); ++it)
			{
				if (it->GetPath() == path)
				{
					eraseAll(m_UndoDirectories, *it);
					eraseAll(m_RedoDirectories, *it);
					parentFile->RemoveFile(it);
					break;
				}
			}
			m_CurrentFile = findFile(currentNodePath, m_Root);
			m_LeftClickedFile = nullptr;
			m_LeftDoubleClickedFile = nullptr;
			m_RightClickedFile = nullptr;
		}

		void ImGuiFileManager::SetCurrentFile(const std::filesystem::path& path)
		{
			setCurrentFile(*findFile(path, m_Root));
		}
	
		void ImGuiFileManager::RenderCurrentDirectory(const char* dragName, glm::vec2 iconSize)
		{
			const bool mouseClicked =
				ImGui::IsMouseClicked(ImGuiMouseButton_Left)
			 || ImGui::IsMouseClicked(ImGuiMouseButton_Right);

			if (mouseClicked
				&& ImGui::IsWindowFocused()
				&& ImGui::IsWindowHovered())
			{
				m_LeftClickedFile = nullptr;
				m_RightClickedFile = nullptr;
				m_LeftDoubleClickedFile = nullptr;
			}


			static float padding = 32.0f;
			const float  cellSize = iconSize.x + padding;

			const float panelWidth = ImGui::GetContentRegionAvail().x;
			int columnCount = (int)(panelWidth / cellSize);
			columnCount = std::max(1, columnCount);

			ImGui::Columns(columnCount, 0, false);
			for (auto& file : *m_CurrentFile)
			{			
				UI::ScopedID id(file.GetName().c_str());
				ImGuiFile::State state = file.Render(dragName, iconSize);
				if (state == ImGuiFile::State::LeftDoubleClicked)
				{
					m_LeftDoubleClickedFile = &file;
				}
				else if (state == ImGuiFile::State::LeftClicked)
				{
					m_LeftClickedFile = &file;
				}
				else if (state == ImGuiFile::State::RightClicked)
				{
					m_RightClickedFile = &file;
				}
				ImGui::NextColumn();
			}
			ImGui::Columns(1);

			if (m_LeftDoubleClickedFile)
			{
				const auto& ext = m_Extensions[m_LeftDoubleClickedFile->GetExtension()];
				if (ext.LeftDoubleClickAction)
				{
					if (ext.LeftDoubleClickAction(m_LeftDoubleClickedFile->GetPath()))
						m_LeftDoubleClickedFile = nullptr;
				}
			}
			else if (m_LeftClickedFile)
			{
				const auto& ext = m_Extensions[m_LeftClickedFile->GetExtension()];
				if (ext.LeftClickAction)
				{
					if (ext.LeftClickAction(m_LeftClickedFile->GetPath()))
						m_LeftClickedFile = nullptr;
				}
			}
			else if (m_RightClickedFile)
			{
				const auto& ext = m_Extensions[m_RightClickedFile->GetExtension()];
				if (ext.RightClickAction)
				{
					if (ext.RightClickAction(m_RightClickedFile->GetPath()))
						m_RightClickedFile = nullptr;
				}
			}
		}

		void ImGuiFileManager::RenderDirectoryTree()
		{
			if (m_ExtensionsUpdated)
			{
				std::filesystem::path currentNodePath = m_CurrentFile->GetPath();
				processDirectory(m_Root, m_Root.GetPath());
				m_CurrentFile = findFile(currentNodePath, m_Root);
				m_ExtensionsUpdated = false;
			}
			auto it = m_Extensions.find("dir");
			if (it == m_Extensions.end())
				return;

			renderDirectoryTree(m_Root, it->second);			
		}

		void ImGuiFileManager::Undo()
		{
			m_RedoDirectories.push_back(m_CurrentFile);
			m_CurrentFile = m_UndoDirectories.back();
			m_UndoDirectories.pop_back();
		}
		void ImGuiFileManager::Redo()
		{
			m_UndoDirectories.push_back(m_CurrentFile);
			m_CurrentFile = m_RedoDirectories.back();
			m_RedoDirectories.pop_back();
		}
		void ImGuiFileManager::RegisterExtension(const std::string& ext, const FileExtensionInfo& extInfo)
		{
			m_Extensions[ext] = extInfo;
			m_ExtensionsUpdated = true;
		}
		void ImGuiFileManager::setCurrentFile(ImGuiFile& file)
		{
			m_UndoDirectories.push_back(m_CurrentFile);
			m_RedoDirectories.clear();
			m_CurrentFile = &file;
		}
		void ImGuiFileManager::renderDirectoryTree(ImGuiFile& file, const FileExtensionInfo& extInfo)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow;
			if (&file == &m_Root)
				flags |= ImGuiTreeNodeFlags_DefaultOpen;

			if (!file.HasSubdirectories())
				flags |= ImGuiTreeNodeFlags_Leaf;


			if (file.IsDirectory())
			{
				std::string folderID = "##" + file.GetName();
				const bool opened = ImGui::TreeNodeEx(folderID.c_str(), flags);

				if (UI::Utils::IsItemDoubleClicked(ImGuiMouseButton_Left))
				{
					if (extInfo.LeftDoubleClickAction)
						extInfo.LeftDoubleClickAction(file.GetPath());
				}

				ImGui::SameLine();
				UI::Image(extInfo.Texture->GetImage(), { GImGui->Font->FontSize , GImGui->Font->FontSize }, extInfo.TexCoords[0], extInfo.TexCoords[1]);
				ImGui::SameLine();
				ImGui::Text(file.GetName().c_str());

				if (opened)
				{
					for (auto& node : file)
						renderDirectoryTree(node, extInfo);
					ImGui::TreePop();
				}
			}
		}
		void ImGuiFileManager::processDirectory(ImGuiFile& file, const std::filesystem::path& dirPath)
		{
			for (auto& it : std::filesystem::directory_iterator(dirPath))
			{
				auto ext = findExtensionInfo(it.path());
				if (ext)
				{
					ImGuiFile newFile(it.path(), ext->TexCoords, ext->Texture);
					if (it.is_directory())
					{
						processDirectory(newFile, it.path());
					}
					file.AddFile(newFile);
				}
			}
		}
		ImGuiFile* ImGuiFileManager::findFile(const std::filesystem::path& path, ImGuiFile& file) const
		{
			std::stack<ImGuiFile*> nodes;
			nodes.push(&file);
			while (!nodes.empty())
			{
				ImGuiFile* tmp = nodes.top();
				nodes.pop();
				if (tmp->GetPath() == path)
					return tmp;

				for (auto& child : *tmp)
					nodes.push(&child);
			}
			return nullptr;
		}
		const FileExtensionInfo* ImGuiFileManager::findExtensionInfo(const std::filesystem::path& path) const
		{
			if (std::filesystem::is_directory(path))
			{
				auto ext = m_Extensions.find(sc_DirectoryExtension);
				if (ext != m_Extensions.end())
				{
					return &ext->second;
				}
			}
			else
			{
				auto ext = m_Extensions.find(Utils::GetExtension(path.filename().string()));
				if (ext != m_Extensions.end())
				{
					return &ext->second;
				}
			}
			return nullptr;
		}
	}
}