#include "stdafx.h"
#include "AssetManager.h"

namespace XYZ
{
	void AssetManager::LoadShader(const std::string& name, const std::string& path)
	{
		XYZ_ASSERT(m_Shaders.find(name) == m_Shaders.end(), "Shader's already loaded.");
		std::string fullPath = ASSETS_FOLDER + "Shaders/" + path;
		if (m_FreedIds.empty())
		{
			m_Shaders[name] = std::make_pair(m_AssetIdCounter++, XYZ::Shader::Create(name, fullPath));
			m_AssetIdCounter++;
		}
		else
		{
			m_Shaders[name] = std::make_pair(m_FreedIds.front(), XYZ::Shader::Create(name, fullPath));
			m_FreedIds.pop();
		}
	}

	std::shared_ptr<XYZ::Shader> AssetManager::GetShader(const std::string& name) const
	{
		XYZ_ASSERT(m_Shaders.find(name) != m_Shaders.end(), "Shader was not loaded.");
		return m_Shaders.at(name).second;
	}

	void AssetManager::LoadTexture(TextureWrap wrap, const std::string& name, const std::string& path)
	{
		XYZ_ASSERT(m_Textures.find(name) == m_Textures.end(), "Texture is already loaded.");
		std::string fullPath = ASSETS_FOLDER + "Textures/" + path;
		if (m_FreedIds.empty())
		{
			m_Textures[name] = std::make_pair(m_AssetIdCounter, XYZ::Texture2D::Create(wrap, fullPath));
			m_AssetIdCounter++;
		}
		else
		{
			m_Textures[name] = std::make_pair(m_FreedIds.front(), XYZ::Texture2D::Create(wrap, fullPath));
			m_FreedIds.pop();
		}
	}

	std::shared_ptr<XYZ::Texture2D> AssetManager::GetTexture(const std::string& name) const
	{
		XYZ_ASSERT(m_Textures.find(name) != m_Textures.end(), "Shader was not loaded.");
		return m_Textures.at(name).second;
	}

	void AssetManager::UnloadShader(const std::string& name)
	{
		m_FreedIds.push(m_Shaders[name].first);
		m_Shaders.erase(name);
	}

	void AssetManager::UnloadTexture(const std::string& name)
	{
		m_FreedIds.push(m_Textures[name].first);
		m_Textures.erase(name);
	}
}
