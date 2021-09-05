#pragma once

#include "XYZ/Asset/Asset.h"

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

	enum class TextureParam
	{
		None = 0,
		Nearest = 1,
		Linear = 2
	};


	struct TextureSpecs
	{
		TextureWrap  Wrap     = TextureWrap::Clamp;
		TextureParam MinParam = TextureParam::Linear;
		TextureParam MagParam = TextureParam::Nearest;
	};

	/**
	* @interface Texture
	* pure virtual (interface) class.
	*/
	class Texture : public Asset
	{
	public:
		virtual ~Texture() = default;
	
		virtual void Bind(uint32_t slot = 0) const = 0;
		
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetChannels() const = 0;

		virtual uint32_t GetRendererID() const = 0;
		
		static uint32_t CalculateMipMapCount(uint32_t width, uint32_t height);
	};

	/**
	* @class Texture2D
	* @brief class Derived from Texture
	*/
	class Texture2D : public Texture
	{
	public:
		virtual const TextureSpecs& GetSpecification() const = 0;
		virtual void SetData(void* data, uint32_t size) = 0;
		virtual void GetData(uint8_t** buffer) const = 0;
		virtual const std::string& GetFilepath() const = 0;

		static Ref<Texture2D> Create(uint32_t width, uint32_t height, uint32_t channels, const TextureSpecs& specs);
		static Ref<Texture2D> Create(const TextureSpecs& specs, const std::string& path);

		
		static void BindStatic(uint32_t rendererID, uint32_t slot);
	};


	class Texture2DArray : public Texture
	{
	public:
		virtual const TextureSpecs& GetSpecification() const = 0;
		
		static Ref<Texture2D> Create(const TextureSpecs& specs, const std::initializer_list<std::string>& paths);
		static Ref<Texture2DArray> Create(uint32_t layerCount, uint32_t width, uint32_t height, uint32_t channels, const TextureSpecs& specs);
	};
}