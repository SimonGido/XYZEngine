#pragma once

#include "stdafx.h"
#include "Scene.h"
#include "XYZ/Core/Singleton.h"

namespace XYZ
{
	/*! @class RemoveActiveSceneException
	*	@brief Is thrown when a currently active scene is being removed
	*/
	class RemoveActiveSceneException : public std::exception
	{
		std::string m_Exception;
		virtual const char* what() const throw();
	public:
		RemoveActiveSceneException(const std::string& msg) : m_Exception(msg) {}
	};

	/*! @class SceneManager
	*	@brief Contains all Scenes and manages adding and removing scenes and switching between them
	*/
	class SceneManager : public Singleton<SceneManager>
	{	
	public:
		SceneManager(token) {};
		~SceneManager() {};

	
		bool Remove(const std::string& name);

		/** Sets scene as active. This will mark the scene for rendering */
		bool SetActive(const std::string& name);


		inline Ref<Scene> GetActive() const { return m_CurrentScene; }
		
		Ref<Scene> CreateScene(const std::string& name);
	private:
		Ref<Scene> m_CurrentScene;

		std::unordered_map<std::string, Ref<Scene>> m_Scenes;
	};
}