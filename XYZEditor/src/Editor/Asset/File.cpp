#include "stdafx.h"
#include "File.h"

#include "EditorLayer.h"

#include "XYZ/Utils/StringUtils.h"

namespace XYZ {
	namespace Editor {
		File::File(std::filesystem::path path, const UV& texCoords, const Ref<Texture2D>& texture)
			:
			m_Path(path),
			m_TexCoords(texCoords),
			m_Texture(texture)
		{
			m_PathStr = m_Path.string();
			m_Name = m_Path.filename().string();
			m_Extension = Utils::GetExtension(m_PathStr);
			memset(m_NameBuffer, 0, _MAX_FNAME);
		}
		File::State File::OnImGuiRender(const char* dragName, glm::vec2 size)
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
		void File::Rename(std::filesystem::path path)
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
		void File::Delete()
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
		void File::finishEditing()
		{
			m_Name = m_NameBuffer;
			auto newPath = m_Path.parent_path() / m_Name;
			Rename(newPath);
			m_EditingName = false;
			m_FocusedEdit = false;
		}
		FileManager::FileManager(const std::filesystem::path& path)
			:
			m_Root(path, {}, nullptr)
		{
			m_CurrentFile = &m_Root;
			processDirectory(m_Root.m_Files, path);
		}
		
		FileManager::FileManager(FileManager&& other) noexcept
			:
			m_Root(std::move(other.m_Root)),
			m_UndoDirectories(std::move(other.m_UndoDirectories)),
			m_RedoDirectories(std::move(other.m_RedoDirectories))
		{
			m_CurrentFile = findNode(other.m_CurrentFile->m_Path, m_Root);
		}
		
		FileManager& FileManager::operator=(FileManager&& other) noexcept
		{
			m_Root = std::move(other.m_Root);
			m_UndoDirectories = std::move(other.m_UndoDirectories);
			m_RedoDirectories = std::move(other.m_RedoDirectories);
			m_CurrentFile = findNode(other.m_CurrentFile->m_Path, m_Root);
			return *this;
		}
		void FileManager::AddFile(const std::filesystem::path& path)
		{
			std::filesystem::path currentNodePath = m_CurrentFile->m_Path;
			auto ext = findExtensionInfo(path);
			if (ext)
			{
				auto file = findNode(path.parent_path(), m_Root);
				file->m_Files.emplace_back(path, ext->TexCoords, ext->Texture);
			}
			m_CurrentFile = findNode(currentNodePath, m_Root);
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

		void FileManager::RemoveFile(const std::filesystem::path& path)
		{
			std::filesystem::path currentNodePath = m_CurrentFile->m_Path;
			auto parentFile = findNode(path.parent_path(), m_Root);
			for (size_t i = 0; i < parentFile->m_Files.size(); ++i)
			{
				auto& files= parentFile->m_Files;
				if (files[i].m_Path == path)
				{
					eraseAll(m_UndoDirectories, files[i]);
					eraseAll(m_RedoDirectories, files[i]);
					files.erase(files.begin() + i);
					return;
				}
			}
			m_CurrentFile = findNode(currentNodePath, m_Root);
		}

		void FileManager::SetCurrentFile(const std::filesystem::path& path)
		{
			setCurrentFile(*findNode(path, m_Root));
		}
	
		void FileManager::ImGuiRenderCurrentDir(const char* dragName, glm::vec2 iconSize)
		{
			static float padding = 32.0f;
			const float  cellSize = iconSize.x + padding;

			const float panelWidth = ImGui::GetContentRegionAvail().x;
			int columnCount = (int)(panelWidth / cellSize);
			columnCount = std::max(1, columnCount);

			ImGui::Columns(columnCount, 0, false);
			for (auto& file : *m_CurrentFile)
			{			
				UI::ScopedID id(file.GetName().c_str());
				File::State state = file.OnImGuiRender(dragName, iconSize);
				if (state == File::State::LeftDoubleClicked)
				{
					m_LeftDoubleClickedFile = &file;
				}
				else if (state == File::State::LeftClicked)
				{
					m_LeftClickedFile = &file;
				}
				else if (state == File::State::RightClicked)
				{
					m_RightClickedFile = &file;
				}
			}

			if (m_LeftDoubleClickedFile)
			{
				const auto& ext = m_Extensions[m_LeftDoubleClickedFile->m_Extension];
				if (ext.LeftDoubleClickAction(m_LeftDoubleClickedFile->GetPath()))
					m_LeftDoubleClickedFile = nullptr;
			}
			else if (m_LeftClickedFile)
			{
				const auto& ext = m_Extensions[m_LeftClickedFile->m_Extension];
				if (ext.LeftClickAction(m_LeftClickedFile->GetPath()))
					m_LeftClickedFile = nullptr;
			}
			else if (m_RightClickedFile)
			{
				const auto& ext = m_Extensions[m_RightClickedFile->m_Extension];
				if (ext.RightClickAction(m_RightClickedFile->GetPath()))
					m_RightClickedFile = nullptr;
			}
		}

		void FileManager::ImGuiRenderDirTree()
		{
			auto it = m_Extensions.find("dir");
			if (it == m_Extensions.end())
				return;

			imGuiRenderDirTree(m_Root, it->second);
		}

		void FileManager::Undo()
		{
			m_RedoDirectories.push_back(m_CurrentFile);
			m_CurrentFile = m_UndoDirectories.back();
			m_UndoDirectories.pop_back();
		}
		void FileManager::Redo()
		{
			m_UndoDirectories.push_back(m_CurrentFile);
			m_CurrentFile = m_RedoDirectories.back();
			m_RedoDirectories.pop_back();
		}
		void FileManager::RegisterExtension(const std::string& ext, const FileExtensionInfo& extInfo)
		{
			m_Extensions[ext] = extInfo;
		}
		void FileManager::setCurrentFile(File& file)
		{
			m_UndoDirectories.push_back(m_CurrentFile);
			m_RedoDirectories.clear();
			m_CurrentFile = &file;
		}
		void FileManager::imGuiRenderDirTree(File& file, const FileExtensionInfo& extInfo)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow;
			if (&file == &m_Root)
				flags |= ImGuiTreeNodeFlags_DefaultOpen;

			if (file.Empty())
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
						imGuiRenderDirTree(node, extInfo);
					ImGui::TreePop();
				}
			}
		}
		void FileManager::processDirectory(std::vector<File>& files, const std::filesystem::path& dirPath)
		{
			for (auto& it : std::filesystem::directory_iterator(dirPath))
			{
				auto ext = findExtensionInfo(it.path());
				if (ext)
				{
					files.emplace_back(it.path(), ext->TexCoords, ext->Texture);
					if (it.is_directory())
					{
						processDirectory(files.back().m_Files, it.path());
					}
				}
			}
		}
		File* FileManager::findNode(const std::filesystem::path& path, File& file) const
		{
			std::stack<File*> nodes;
			nodes.push(&file);
			while (!nodes.empty())
			{
				File* tmp = nodes.top();
				nodes.pop();
				if (tmp->m_Path == path)
					return tmp;

				for (auto& child : *tmp)
					nodes.push(&child);
			}
			return nullptr;
		}
		const FileExtensionInfo* FileManager::findExtensionInfo(const std::filesystem::path& path) const
		{
			if (std::filesystem::is_directory(path))
			{
				auto ext = m_Extensions.find("dir");
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