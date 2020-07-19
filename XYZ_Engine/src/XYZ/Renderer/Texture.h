#pragma once

#include "XYZ/Core/Ref.h"

#include <string>
#include <memory>


namespace XYZ {
	enum class TextureFormat
	{
		None = 0,
		RGB = 1,
		RGBA = 2,
	};

	enum class TextureWrap
	{
		None = 0,
		Clamp = 1,
		Repeat = 2
	};

	/**
	* @interface Texture
	* pure virtual (interface) class.
	*/
	class Texture : public RefCount
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;
		virtual void SetData(void* data, uint32_t size) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;

	
	};

	/**
	* @class Texture2D
	* @brief class Derived from Texture
	*/
	class Texture2D : public Texture
	{
	public:
		/**
		* @param[in] format Format of the texture
		* @param[in] wrap   Component of wrapping of the texture
		* @param[in] width  Width of the texture
		* @param[in] height Height of the texture
		* @return shared_ptr to empty Texture2D
		*/
		static Ref<Texture2D> Create(TextureFormat format, TextureWrap wrap, uint32_t width, uint32_t height);
		
		
		/**
		* @param[in] wrap   Component of wrapping of the texture
		* @param[in] path	File path to the image
		* @return shared_ptr to Texture2D
		*/
		static Ref<Texture2D> Create(TextureWrap wrap, const std::string& path);

		static void Bind(uint32_t rendererID, uint32_t slot);
	};

}