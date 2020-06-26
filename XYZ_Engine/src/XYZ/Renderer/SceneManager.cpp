#include "stdafx.h"
#include "SceneManager.h"

const char* XYZ::RemoveActiveSceneException::what() const throw()
{
	return ((std::string)"Removing active scene (").append(m_Exception).append(")!").c_str();
}

void XYZ::SceneManager::Add(Scene& scene)
{
	m_Scenes[scene.GetName()] = std::make_shared<Scene>(scene);
}

void XYZ::SceneManager::Add(std::shared_ptr<Scene> scene)
{
	m_Scenes[scene->GetName()] = scene;
}

bool XYZ::SceneManager::Remove(const std::string& name)
{
	if (m_RendererIDticeScene->GetName() == name) throw RemoveActiveSceneException(name);
	if (m_Scenes.count(name) == 0) return false;

	m_Scenes.erase(name);
	return true;
}

bool XYZ::SceneManager::SetActive(const std::string& name)
{
	if (m_Scenes.count(name) == 0) return false;
	m_RendererIDticeScene = m_Scenes[name];
}

void XYZ::SceneManager::AddEntityToActive(const Entity& item)
{
	
}
