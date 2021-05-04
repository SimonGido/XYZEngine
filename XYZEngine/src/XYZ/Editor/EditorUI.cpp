#include "stdafx.h"
#include "EditorUI.h"

#include "XYZ/BasicUI/BasicUILoader.h"
#include "XYZ/Utils/StringUtils.h"

namespace XYZ {
	namespace Editor {
		EditorUI::EditorUI(const std::string& filepath)
			:
			m_Filepath(filepath),
			m_Name(Utils::GetFilenameWithoutExtension(m_Filepath))
		{
			bUILoader::Load(filepath);
			bUI::SetOnReloadCallback(m_Name, [&](bUIAllocator& allocator) {
				OnReload();
			});
		}
		EditorUI::~EditorUI()
		{
			bUILoader::Save(m_Name, m_Filepath.c_str());
		}
	}
}