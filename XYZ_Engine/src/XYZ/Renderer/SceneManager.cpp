#include "stdafx.h"
#include "SceneManager.h"



namespace XYZ {


	const char* RemoveActiveSceneException::what() const throw()
	{
		return ((std::string)"Removing active scene (").append(m_Exception).append(")!").c_str();
	}

	void SceneManager::Add(Scene& scene)
	{
		m_Scenes[scene.GetName()] = std::make_shared<Scene>(scene);
	}

	void SceneManager::Add(Ref<Scene> scene)
	{
		m_Scenes[scene->GetName()] = scene;
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
		m_CurrentScene = m_Scenes[name];
	}

	uint16_t SceneManager::AddObjectToActive(const SceneObject& object)
	{
		return m_CurrentScene->AddObject(object);
	}

}