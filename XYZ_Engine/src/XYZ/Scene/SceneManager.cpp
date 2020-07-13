#include "stdafx.h"
#include "SceneManager.h"



namespace XYZ {


	const char* RemoveActiveSceneException::what() const throw()
	{
		return ((std::string)"Removing active scene (").append(m_Exception).append(")!").c_str();
	}


	Ref<Scene> SceneManager::CreateScene(const std::string& name)
	{
		m_Scenes.insert({ name,Ref<Scene>::Create(name) });
		return m_Scenes[name];
	}

	bool SceneManager::Remove(const std::string& name)
	{
		if (m_CurrentScene->GetName() == name) throw RemoveActiveSceneException(name);
		if (m_Scenes.count(name) == 0) return false;

		m_Scenes.erase(name);
		return true;
	}

	bool SceneManager::SetActive(const std::string& name)
	{
		if (m_Scenes.count(name) == 0) return false;
		m_CurrentScene->OnDetach();
		m_CurrentScene = m_Scenes[name];
		m_CurrentScene->OnAttach();
	}
}