#pragma once

#include "stdafx.h"
#include "Scene.h"

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
	class SceneManager
	{	
	public:
		SceneManager() {};
		~SceneManager() {};

		void Add(Scene& scene);
		void Add(std::shared_ptr<Scene> scene);
		bool Remove(const std::string& name);

		/** Sets scene as active. This will mark the scene for rendering */
		bool SetActive(const std::string& name);

		/** Adds an entity into the active scene */
		void AddEntityToActive(const Entity& item);

		inline std::shared_ptr<Scene> GetActive() const { return m_RendererIDticeScene; }


	private:
		std::shared_ptr<Scene> m_RendererIDticeScene;
		std::unordered_map<std::string, std::shared_ptr<Scene>> m_Scenes;
	};
}