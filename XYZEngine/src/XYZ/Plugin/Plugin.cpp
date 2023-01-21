#include "stdafx.h"
#include "Plugin.h"

namespace XYZ {
	Plugin::Plugin(std::filesystem::path path)
		:
		m_Filepath(std::move(path))
	{
	}
}