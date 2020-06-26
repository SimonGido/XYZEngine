#pragma once

#include <unordered_map>
#include <queue>
#include "XYZ/Renderer/Shader.h"
#include "XYZ/Renderer/Texture.h"

#include "XYZ/Core/Singleton.h"

namespace XYZ
{
	/*! @class AssetManager
	*	@brief The class to manage game assets
	*/
	class AssetManager : public Singleton<AssetManager>
	{
		/** Path to the folder with assets */
		const std::string ASSETS_FOLDER = "Assets/";

		/** Shaders collection */
		std::unordered_map<std::string, std::pair<uint16_t, std::shared_ptr<XYZ::Shader>>> m_Shaders;
		/** Textures collection */
		std::unordered_map<std::string, std::pair<uint16_t, std::shared_ptr<XYZ::Texture2D>>> m_Textures;

		/** Counter to track next free id to assign to an asset */
		uint16_t m_AssetIdCounter;
		/** Queue with ids of uloaded assets */
		std::queue<uint16_t> m_FreedIds;

	public:	
		AssetManager(token) :m_AssetIdCounter(0) {};
		/**
		*	Creates a shader by given path and adds it into collection of shaders
		*	@param name Shader name to be stored in the collection
		*	@param path Path to the shader
		*/
		void LoadShader(const std::string&, const std::string&);
		/**
		*	Remove a shader from the collection
			@param name Shader name to be removed
		*/
		void UnloadShader(const std::string&);
		/**
		*	Return shared_ptr to a shader from the collection
		*	@param name Shader name
		*	@return shared_ptr to XYZ::Shader instance
		*/
		std::shared_ptr<XYZ::Shader> GetShader(const std::string&) const;

		/**
		*	Creates a texture by given path and adds it into collection of textures
		*	@param name Texture name to be stored in the collection
		*	@param path Path to the texture
		*/
		void LoadTexture(TextureWrap, const std::string&, const std::string&);
		/**
		*	Remove a texture from the collection
			@param name Texture name to be removed
		*/
		void UnloadTexture(const std::string&);
		/**
		*	Return shared_ptr to a texture from the collection stored as XYZ::Texture2D
		*	@param name Texture name
		*	@return shared_ptr to XYZ::Texture instance
		*/
		std::shared_ptr<XYZ::Texture2D> GetTexture(const std::string&) const;

	};
}
